#include "renderer.hpp"

#include <format>

#include "../../modules/windowing/windowing.hpp"
#include "../../modules/resources/resources.hpp"

#include <iostream>

//--------------------------------------------------------------------------------
// Init / Deinit
//--------------------------------------------------------------------------------

void WINDUP_Rendering::init(WINDUP_EngineConfigs& arg_global_configs, WINDUP_Threading& arg_threading, WINDUP_Devices& arg_devices, WINDUP_Resources& arg_resources, WINDUP_Windowing& arg_windowing)
{
	engine_configs = &arg_global_configs;

	devices = &arg_devices;
	resources = &arg_resources;
	windowing = &arg_windowing;

	WINDUP_Window *window = arg_windowing.get_window();

	// load_configs(*resources, "configs/configs_renderer.json");

	bind_window_to_gpu_device();
	create_depth_texture();
	create_sampler();
	create_fallback_texture();

	init_gui();

	status.f_is_init = true;

	WINDUP_Logger::task_result("Rendering", "Initialization", status.f_is_init);
}

void WINDUP_Rendering::deinit()
{
	WINDUP_GPUDevice *gpu_device = devices->get_gpu_device();

	SDL_WaitForGPUIdle(gpu_device);

	ImGui_ImplSDLGPU3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	for (auto &[handle, pipeline] : pipelines_cache)
	{
		if (pipeline.sdl_pipeline) SDL_ReleaseGPUGraphicsPipeline(gpu_device, pipeline.sdl_pipeline);
	}
	pipelines_cache.clear();

	if (curr_renderer_resources.default_sampler)    SDL_ReleaseGPUSampler(gpu_device, curr_renderer_resources.default_sampler);
	if (curr_renderer_resources.fallback_texture)    SDL_ReleaseGPUTexture(gpu_device, curr_renderer_resources.fallback_texture);
	if (curr_renderer_resources.depth_texture)       SDL_ReleaseGPUTexture(gpu_device, curr_renderer_resources.depth_texture);
	if (curr_renderer_resources.scene_target.color)  SDL_ReleaseGPUTexture(gpu_device, curr_renderer_resources.scene_target.color);
	if (curr_renderer_resources.scene_target.depth)  SDL_ReleaseGPUTexture(gpu_device, curr_renderer_resources.scene_target.depth);

	status.f_is_deinit = true;

	WINDUP_Logger::task_result("Rendering", "Deinitialization", status.f_is_deinit);
}

//--------------------------------------------------------------------------------
// Pipelines
//--------------------------------------------------------------------------------

WINDUP_PipelineHandle WINDUP_Rendering::create_pipeline(WINDUP_PipelineDesc desc)
{
	WINDUP_GPUDevice *gpu_device = devices->get_gpu_device();
	WINDUP_Window *window = windowing->get_window();

	if (!resources->has_shader(desc.shader_name))
	{
		WINDUP_Logger::error("Rendering", "Shader not found: " + desc.shader_name, 0);
		return {}; // invalid handle
	}
	WINDUP_Shader &shader = resources->get_shader(desc.shader_name);

	if (!shader.vert || !shader.frag)
	{
		WINDUP_Logger::error("Rendering", "Shader has null vert or frag", 0);
	}

	// Vertex input (hardcoded for E_Vertex)
	SDL_GPUVertexBufferDescription vb_desc{};
	vb_desc.slot = 0;
	vb_desc.pitch = sizeof(WINDUP_Vertex);
	vb_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

	SDL_GPUVertexAttribute attribs[] = {
		{
			.location = 0, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = offsetof(WINDUP_Vertex, position)
		},
		{
			.location = 1, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = offsetof(WINDUP_Vertex, normal)
		},
		{
			.location = 2, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
			.offset = offsetof(WINDUP_Vertex, uv)
		},
	};

	// Input stage
	SDL_GPUVertexInputState vert_input{};
	vert_input.vertex_buffer_descriptions = &vb_desc;
	vert_input.num_vertex_buffers = 1;
	vert_input.vertex_attributes = attribs;
	vert_input.num_vertex_attributes = 3;

	// Color target stage
	SDL_GPUColorTargetDescription color_desc{};
	color_desc.format = SDL_GetGPUSwapchainTextureFormat(gpu_device, window);

	// Depth/stencil stage
	SDL_GPUDepthStencilState depth_stencil{};
	depth_stencil.enable_depth_test = desc.raster_depth_configs.depth_test_enable;
	depth_stencil.enable_depth_write = desc.raster_depth_configs.depth_write_enable;
	depth_stencil.compare_op = desc.raster_depth_configs.depth_compare_op;

	// Rasterization stage
	SDL_GPURasterizerState rasterizer{};
	rasterizer.fill_mode = desc.raster_depth_configs.fill_mode;
	rasterizer.cull_mode = desc.raster_depth_configs.cull_mode;
	rasterizer.front_face = desc.raster_depth_configs.front_face;

	// Pipeline creation
	SDL_GPUGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.vertex_shader = shader.vert;
	pipeline_info.fragment_shader = shader.frag;
	pipeline_info.primitive_type = desc.primitive_type;
	pipeline_info.target_info.color_target_descriptions = &color_desc;
	pipeline_info.target_info.num_color_targets = 1;
	pipeline_info.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	pipeline_info.target_info.has_depth_stencil_target = true;
	pipeline_info.depth_stencil_state = depth_stencil;
	pipeline_info.rasterizer_state = rasterizer;
	pipeline_info.vertex_input_state = vert_input;

	SDL_GPUGraphicsPipeline *sdl_pipeline = SDL_CreateGPUGraphicsPipeline(gpu_device, &pipeline_info);

	WINDUP_Pipeline pipeline;
	pipeline.desc = desc;
	pipeline.sdl_pipeline = sdl_pipeline;

	WINDUP_PipelineHandle handle = pipelines_cache.cache_object(pipeline.desc.name, pipeline);
	pipelines_cache.get_by_handle(handle).handle = handle;

	return handle;
}

WINDUP_PipelineHandle WINDUP_Rendering::get_pipeline(const std::string &name)
{
	WINDUP_PipelineHandle handle = pipelines_cache.get_by_name(name).handle;
	return handle;
}

void WINDUP_Rendering::use_pipeline(WINDUP_PipelineHandle handle)
{
	WINDUP_Pipeline &pipeline = pipelines_cache.get_by_handle(handle);
	curr_frame_state.pipeline = pipeline.sdl_pipeline;
	SDL_BindGPUGraphicsPipeline(curr_frame_state.render_pass, curr_frame_state.pipeline);
}

void WINDUP_Rendering::use_pipeline(const std::string &name)
{
	WINDUP_Pipeline &pipeline = pipelines_cache.get_by_name(name);
	curr_frame_state.pipeline = pipeline.sdl_pipeline;
	SDL_BindGPUGraphicsPipeline(curr_frame_state.render_pass, curr_frame_state.pipeline);
}

bool WINDUP_Rendering::destroy_pipeline(WINDUP_PipelineHandle handle)
{
	if (!pipelines_cache.has(handle)) return true;
	WINDUP_Pipeline& pipeline = pipelines_cache.get_by_handle(handle);
	if (pipeline.sdl_pipeline) SDL_ReleaseGPUGraphicsPipeline(devices->get_gpu_device(), pipeline.sdl_pipeline);
	pipelines_cache.remove(handle);
	return true;
}

//--------------------------------------------------------------------------------
// Frame lifecycle
//--------------------------------------------------------------------------------

bool WINDUP_Rendering::begin_frame()
{
	WINDUP_GPUDevice *gpu_device = devices->get_gpu_device();
	WINDUP_Window *window = windowing->get_window();

	curr_frame_state.cmd_buffer = SDL_AcquireGPUCommandBuffer(gpu_device);

	SDL_WaitAndAcquireGPUSwapchainTexture(curr_frame_state.cmd_buffer,
										  window,
										  &curr_frame_state.swapchain,
										  NULL,
										  NULL);

	if (!curr_frame_state.swapchain)
	{
		SDL_CancelGPUCommandBuffer(curr_frame_state.cmd_buffer);
		curr_frame_state.cmd_buffer = nullptr;
		return false;
	}

	// ImGui lifecycle — always runs, pairs with Render() in prepare_ui()
	ImGui_ImplSDLGPU3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	return true;
}

void WINDUP_Rendering::end_frame()
{
	SDL_SubmitGPUCommandBuffer(curr_frame_state.cmd_buffer);
	curr_frame_state.cmd_buffer = nullptr;
	curr_frame_state.swapchain = nullptr;
	curr_frame_state.imgui_draw_data = nullptr;
}

//--------------------------------------------------------------------------------
// Scenes
//--------------------------------------------------------------------------------

void WINDUP_Rendering::create_scene_target(uint32_t w, uint32_t h)
{
	WINDUP_GPUDevice *gpu_device = devices->get_gpu_device();
	WINDUP_Window *window = windowing->get_window();
	if (!gpu_device)
	{
		WINDUP_Logger::error("Rendering", "create_scene_target: no GPU device", 0);
		return;
	}

	if (w == 0 || h == 0)
	{
		// panel collapsed or not laid out yet — skip this frame
		return;
	}

	WINDUP_SceneTarget &target = curr_renderer_resources.scene_target;

	// Already the right size — nothing to do.
	if (target.color && target.w == w && target.h == h)
	{
		return;
	}

	// Tolerate small jitter — only recreate on a real size change.
	if (target.color)
	{
		int dw = (int) w - (int) target.w;
		int dh = (int) h - (int) target.h;
		if (dw >= -2 && dw <= 2 && dh >= -2 && dh <= 2)
			return;
	}

	if (target.depth)
	{
		SDL_ReleaseGPUTexture(gpu_device, target.depth);
		target.depth = nullptr;
	}

	// Color target — COLOR_TARGET so the scene pass draws into it,
	// SAMPLER so ImGui can read it for ImGui::Image().
	SDL_GPUTextureCreateInfo color_info{};
	color_info.type = SDL_GPU_TEXTURETYPE_2D;
	color_info.format = SDL_GetGPUSwapchainTextureFormat(gpu_device, window);
	color_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET |
	                   SDL_GPU_TEXTUREUSAGE_SAMPLER;
	color_info.width = w;
	color_info.height = h;
	color_info.layer_count_or_depth = 1;
	color_info.num_levels = 1;

	target.color = SDL_CreateGPUTexture(gpu_device, &color_info);
	if (!target.color)
	{
		WINDUP_Logger::error("Rendering", SDL_GetError(), 0);
		return;
	}

	// Depth target — D32_FLOAT, panel-sized, matches the color target.
	SDL_GPUTextureCreateInfo depth_info{};
	depth_info.type = SDL_GPU_TEXTURETYPE_2D;
	depth_info.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	depth_info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
	depth_info.width = w;
	depth_info.height = h;
	depth_info.layer_count_or_depth = 1;
	depth_info.num_levels = 1;

	target.depth = SDL_CreateGPUTexture(gpu_device, &depth_info);
	if (!target.depth)
	{
		WINDUP_Logger::error("Rendering", SDL_GetError(), 0);
		SDL_ReleaseGPUTexture(gpu_device, target.color);
		target.color = nullptr;
		return;
	}

	target.w = w;
	target.h = h;
}

//--------------------------------------------------------------------------------
// Meshes
//--------------------------------------------------------------------------------

void WINDUP_Rendering::upload_mesh(WINDUP_MeshHandle mesh_handle)
{
	WINDUP_GPUDevice *gpu_device = devices->get_gpu_device();

	WINDUP_Mesh &cached_mesh = resources->get_mesh(mesh_handle);

	uint32_t vert_size = cached_mesh.desc.vertices.size() * sizeof(WINDUP_Vertex);
	uint32_t idx_size = cached_mesh.desc.indices.size() * sizeof(uint32_t);

	if (vert_size == 0 || idx_size == 0)
	{
		WINDUP_Logger::error("Rendering", "upload_mesh: '"
		                              + cached_mesh.desc.name + "' has no vertices or indices, skipping", 0);
		return;
	}

	// Vertex buffer
	SDL_GPUBufferCreateInfo vb_info{};
	vb_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	vb_info.size = vert_size;
	cached_mesh.vert_buffer = SDL_CreateGPUBuffer(gpu_device, &vb_info);
	if (!cached_mesh.vert_buffer)
	{
		WINDUP_Logger::error("Rendering", std::string("vertex buffer failed: ") + SDL_GetError(), 0);
		return;
	}

	// Index buffer
	SDL_GPUBufferCreateInfo ib_info{};
	ib_info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
	ib_info.size = idx_size;
	cached_mesh.idx_buffer = SDL_CreateGPUBuffer(gpu_device, &ib_info);
	if (!cached_mesh.idx_buffer)
	{
		WINDUP_Logger::error("Rendering", std::string("index buffer failed: ") + SDL_GetError(), 0);
		return;
	}

	// Transfer buffer for both
	SDL_GPUTransferBufferCreateInfo tb_info{};
	tb_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	tb_info.size = vert_size + idx_size;
	SDL_GPUTransferBuffer *tb = SDL_CreateGPUTransferBuffer(gpu_device, &tb_info);
	if (!tb)
	{
		WINDUP_Logger::error("Rendering", std::string("mesh transfer buffer (")
		                              + std::to_string(vert_size + idx_size) + " bytes) failed: " + SDL_GetError(), 0);
		return;
	}

	uint8_t *mapped = (uint8_t *) SDL_MapGPUTransferBuffer(gpu_device, tb, false);
	if (!mapped)
	{
		WINDUP_Logger::error("Rendering", std::string("mesh map failed: ") + SDL_GetError(), 0);
		SDL_ReleaseGPUTransferBuffer(gpu_device, tb);
		return;
	}


	memcpy(mapped, cached_mesh.desc.vertices.data(), vert_size);
	memcpy(mapped + vert_size, cached_mesh.desc.indices.data(), idx_size);
	SDL_UnmapGPUTransferBuffer(gpu_device, tb);

	// Single command buffer and submit
	SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu_device);
	SDL_GPUCopyPass *pass = SDL_BeginGPUCopyPass(cmd);

	SDL_GPUTransferBufferLocation vert_src{.transfer_buffer = tb, .offset = 0};
	SDL_GPUBufferRegion vert_dst{.buffer = cached_mesh.vert_buffer, .offset = 0, .size = vert_size};
	SDL_UploadToGPUBuffer(pass, &vert_src, &vert_dst, false);

	SDL_GPUTransferBufferLocation idx_src{.transfer_buffer = tb, .offset = vert_size};
	SDL_GPUBufferRegion idx_dst{.buffer = cached_mesh.idx_buffer, .offset = 0, .size = idx_size};
	SDL_UploadToGPUBuffer(pass, &idx_src, &idx_dst, false);

	SDL_EndGPUCopyPass(pass);
	SDL_SubmitGPUCommandBuffer(cmd);
	SDL_ReleaseGPUTransferBuffer(gpu_device, tb);
}

//--------------------------------------------------------------------------------
// Models
//--------------------------------------------------------------------------------

void WINDUP_Rendering::upload_model(WINDUP_ModelHandle model_handle)
{
	WINDUP_GPUDevice *gpu_device = devices->get_gpu_device();
	WINDUP_Window *window = windowing->get_window();

	WINDUP_Model &cached_model = resources->get_model(model_handle);

	WINDUP_Logger::info("Uploading model '%s'", cached_model.desc.name.c_str(), 0);

	for (size_t i = 0; i < cached_model.desc.mesh_handles.size(); i++)
	{
		auto &cached_mesh_handle = cached_model.desc.mesh_handles[i];
		WINDUP_Mesh &cached_mesh = resources->get_mesh(cached_mesh_handle);

		WINDUP_Logger::info("Uploading mesh '%s'", cached_mesh.desc.name.c_str(), 0);

		if (cached_mesh.desc.material_handle.valid())
		{
			upload_material(cached_mesh.desc.material_handle);
		} else
		{
			WINDUP_Logger::info("Skipping material upload for mesh %zu (no material)", std::to_string(i), 0);
		}

		upload_mesh(cached_mesh_handle);
	}
}

//--------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------

void WINDUP_Rendering::upload_texture(WINDUP_TextureHandle texture_handle)
{
	if (!texture_handle.valid()) return;

	WINDUP_GPUDevice *gpu_device = devices->get_gpu_device();

	WINDUP_Texture &cached_texture = resources->get_texture(texture_handle);

	if (cached_texture.desc.image_data.empty()) return;

	uint32_t w = cached_texture.desc.width;
	uint32_t h = cached_texture.desc.height;

	if (w == 0 || h == 0)
	{
		WINDUP_Logger::error("Rendering",
		                 "upload_texture: texture '" + cached_texture.desc.name
		                 + "' has zero-size image, skipping", 0);
		return;
	}

	uint32_t image_size = w * h * 4; // RGBA

	SDL_GPUTextureCreateInfo tex_info{};
	tex_info.type = SDL_GPU_TEXTURETYPE_2D;
	tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	tex_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
	tex_info.width = w;
	tex_info.height = h;
	tex_info.layer_count_or_depth = 1;
	tex_info.num_levels = 1;

	cached_texture.texture = SDL_CreateGPUTexture(gpu_device, &tex_info);
	if (!cached_texture.texture)
	{
		WINDUP_Logger::error("Rendering", SDL_GetError(), 0);
		return;
	}

	SDL_GPUTransferBufferCreateInfo tb_info{};
	tb_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	tb_info.size = image_size;
	SDL_GPUTransferBuffer *tb = SDL_CreateGPUTransferBuffer(gpu_device, &tb_info);

	void *mapped = SDL_MapGPUTransferBuffer(gpu_device, tb, false);
	if (!mapped)
	{
		WINDUP_Logger::error("Rendering", std::string("map failed: ") + SDL_GetError(), 0);
		SDL_ReleaseGPUTransferBuffer(gpu_device, tb);
		return;
	}
	memcpy(mapped, cached_texture.desc.image_data.data(), image_size);
	SDL_UnmapGPUTransferBuffer(gpu_device, tb);

	SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu_device);
	SDL_GPUCopyPass *pass = SDL_BeginGPUCopyPass(cmd);

	SDL_GPUTextureTransferInfo src{.transfer_buffer = tb, .offset = 0};
	SDL_GPUTextureRegion dst{};
	dst.texture = cached_texture.texture;
	dst.w = w;
	dst.h = h;
	dst.d = 1;

	SDL_UploadToGPUTexture(pass, &src, &dst, false);
	SDL_EndGPUCopyPass(pass);
	SDL_SubmitGPUCommandBuffer(cmd);
	SDL_ReleaseGPUTransferBuffer(gpu_device, tb);

	cached_texture.gpu_sampler = curr_renderer_resources.default_sampler;
}

void WINDUP_Rendering::create_depth_texture()
{
	// Load GPU
	WINDUP_GPUDevice *gpu_device = devices->get_gpu_device();
	if (!gpu_device)
	{
		WINDUP_Logger::error("Rendering", "create_depth_texture: no GPU device", 0);
		return;
	}

	// Getting window dimensions
	auto [w, h] = windowing->get_window_dimensions();
	if (w <= 0 || h <= 0)
	{
		// window minimized or not ready — skip depth target creation this frame
		WINDUP_Logger::error("Rendering", "Window not ready, skipping depth texture creation", 0);
		return;
	}

	// Release before reuse
	if (curr_renderer_resources.depth_texture)
	{
		SDL_ReleaseGPUTexture(gpu_device, curr_renderer_resources.depth_texture);
		curr_renderer_resources.depth_texture = nullptr;
	}

	// Create depth info
	SDL_GPUTextureCreateInfo depth_info{};
	depth_info.type = SDL_GPU_TEXTURETYPE_2D;
	depth_info.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	depth_info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
	depth_info.width = (uint32_t) w;
	depth_info.height = (uint32_t) h;
	depth_info.layer_count_or_depth = 1;
	depth_info.num_levels = 1;

	// SDL create texture
	curr_renderer_resources.depth_texture = SDL_CreateGPUTexture(gpu_device, &depth_info);
	if (!curr_renderer_resources.depth_texture)
	{
		WINDUP_Logger::error("Rendering", SDL_GetError(), 0);
	};
}

void WINDUP_Rendering::create_fallback_texture()
{
	// Load GPU
	WINDUP_GPUDevice *gpu_device = devices->get_gpu_device();
	if (!gpu_device)
	{
		WINDUP_Logger::error("Rendering", "create_sampler(): no GPU device", 0);
		return;
	};

	// Create texture info
	SDL_GPUTextureCreateInfo tex_info{};
	tex_info.type = SDL_GPU_TEXTURETYPE_2D;
	tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	tex_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
	tex_info.width = 1;
	tex_info.height = 1;
	tex_info.layer_count_or_depth = 1;
	tex_info.num_levels = 1;

	// Create SDL texture
	curr_renderer_resources.fallback_texture = SDL_CreateGPUTexture(gpu_device, &tex_info);
	if (!curr_renderer_resources.fallback_texture)
	{
		WINDUP_Logger::error("Rendering", SDL_GetError(), 0);
		return;
	};

	uint8_t white[4] = {255, 255, 255, 255};

	// Create transfer buffer info
	SDL_GPUTransferBufferCreateInfo tb_info{};
	tb_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	tb_info.size = 4;

	// Create SDL transfer buffer
	SDL_GPUTransferBuffer *tb = SDL_CreateGPUTransferBuffer(gpu_device, &tb_info);
	if (!tb)
	{
		WINDUP_Logger::error("Rendering", SDL_GetError(), 0);
		return;
	};

	void *mapped = SDL_MapGPUTransferBuffer(gpu_device, tb, false);
	if (!mapped)
	{
		WINDUP_Logger::error("Rendering", SDL_GetError(), 0);
		SDL_ReleaseGPUTransferBuffer(gpu_device, tb);
		return;
	}

	memcpy(mapped, white, 4);
	SDL_UnmapGPUTransferBuffer(gpu_device, tb);

	SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu_device);
	SDL_GPUCopyPass *pass = SDL_BeginGPUCopyPass(cmd);

	SDL_GPUTextureTransferInfo src{.transfer_buffer = tb, .offset = 0};
	SDL_GPUTextureRegion dst{};
	dst.texture = curr_renderer_resources.fallback_texture;
	dst.w = 1;
	dst.h = 1;
	dst.d = 1;

	// SDL uploads
	SDL_UploadToGPUTexture(pass, &src, &dst, false);
	SDL_EndGPUCopyPass(pass);
	SDL_SubmitGPUCommandBuffer(cmd);
	SDL_ReleaseGPUTransferBuffer(gpu_device, tb);
}

//--------------------------------------------------------------------------------
// Materials
//--------------------------------------------------------------------------------

void WINDUP_Rendering::upload_material(WINDUP_MaterialHandle material_handle)
{
	if (!material_handle.valid()) return;

	WINDUP_Material &cached_material = resources->get_material(material_handle);
	upload_texture(cached_material.desc.base_color_texture);
}

//--------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------

void WINDUP_Rendering::create_sampler()
{
	// Load GPU
	WINDUP_GPUDevice *gpu_device = devices->get_gpu_device();
	if (!gpu_device)
	{
		WINDUP_Logger::error("Rendering", "create_sampler(): no GPU device", 0);
		return;
	};

	// Create sampler info
	SDL_GPUSamplerCreateInfo smpl_info{};
	smpl_info.min_filter = SDL_GPU_FILTER_LINEAR;
	smpl_info.mag_filter = SDL_GPU_FILTER_LINEAR;
	smpl_info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
	smpl_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
	smpl_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
	smpl_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;

	// SDL create sampler
	curr_renderer_resources.default_sampler = SDL_CreateGPUSampler(gpu_device, &smpl_info);
	if (!curr_renderer_resources.default_sampler)
	{
		WINDUP_Logger::error("Rendering", SDL_GetError(), 0);
		return;
	}
}

//--------------------------------------------------------------------------------
// GUI
//--------------------------------------------------------------------------------

bool WINDUP_Rendering::init_gui()
{
	WINDUP_GPUDevice *gpu_device = devices->get_gpu_device();
	WINDUP_Window *window = windowing->get_window();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	if (!ImGui_ImplSDL3_InitForSDLGPU(window))
	{
		WINDUP_Logger::error("GUI", "ImGui_ImplSDL3_InitForSDLGPU failed", 0);
		return false;
	}

	SDL_GPUTextureFormat color_format = SDL_GetGPUSwapchainTextureFormat(gpu_device, window);
	if (color_format == SDL_GPU_TEXTUREFORMAT_INVALID)
	{
		WINDUP_Logger::error("GUI",
						 "swapchain format INVALID — renderer must claim the window before GUI init", 0);
		return false;
	}

	ImGui_ImplSDLGPU3_InitInfo imgui_init_info{};
	imgui_init_info.Device = gpu_device;
	imgui_init_info.ColorTargetFormat = color_format;
	imgui_init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;

	if (!ImGui_ImplSDLGPU3_Init(&imgui_init_info))
	{
		WINDUP_Logger::error("GUI", "ImGui_ImplSDLGPU3_Init failed", 0);
		return false;
	}

	return true;
}

void WINDUP_Rendering::prepare_gui()
{
	if (engine_configs->enable_editor_mode)
	{
		execute_editor_gui_cmds();
	}
	else
	{
		execute_app_gui_cmds();
	}

	ImGui::Render();
	curr_frame_state.imgui_draw_data = ImGui::GetDrawData();
	ImGui_ImplSDLGPU3_PrepareDrawData(curr_frame_state.imgui_draw_data,
									  curr_frame_state.cmd_buffer);
}

void WINDUP_Rendering::create_app_gui(WINDUP_GUIDesc arg_gui_desc)
{
	app_ui_desc = arg_gui_desc;
}

void WINDUP_Rendering::create_editor_gui(WINDUP_GUIDesc arg_gui_desc)
{
	editor_ui_desc = arg_gui_desc;
}

bool WINDUP_Rendering::load_editor_background(const std::string& filename)
{
	renderer_configs.editor_background = filename;
	return true;
}

void WINDUP_Rendering::apply_theme(ImGuiStyle &style)
{
	ImGui::GetStyle() = style;
}

void WINDUP_Rendering::execute_editor_gui_cmds()
{
	apply_theme(editor_ui_desc.style);
	if (editor_gui_cmds) editor_gui_cmds();
}

void WINDUP_Rendering::execute_app_gui_cmds()
{
	apply_theme(app_ui_desc.style);
	if (app_gui_cmds) app_gui_cmds();
}

//--------------------------------------------------------------------------------
// Draw queue
//--------------------------------------------------------------------------------

bool WINDUP_Rendering::submit_draw_call(WINDUP_DrawCall draw_call)
{
	render_queue.push_back(draw_call);
	return true;
}

bool WINDUP_Rendering::execute_render_queue()
{
	if (render_queue.empty() || !curr_frame_state.render_pass)
	{
		render_queue.clear();
		return true;
	}

	// Bind the mesh pipeline once before drawing
	WINDUP_PipelineHandle mesh_pipeline = get_pipeline("mesh");
	use_pipeline(mesh_pipeline);

	for (auto &draw_call: render_queue)
	{
		WINDUP_Mesh &mesh = resources->get_mesh(draw_call.mesh_handle);
		draw_base(mesh, draw_call.mvp);
	}

	render_queue.clear();
	return true;
}

//--------------------------------------------------------------------------------
// Draw
//--------------------------------------------------------------------------------

void WINDUP_Rendering::draw_base(const WINDUP_Mesh &mesh, const WINDUP_MVP &mvp)
{
	WINDUP_GPUDevice *gpu_device = devices->get_gpu_device();
	WINDUP_Window *window = windowing->get_window();

	SDL_GPUBufferBinding vb{.buffer = mesh.vert_buffer, .offset = 0};
	SDL_GPUBufferBinding ib{.buffer = mesh.idx_buffer, .offset = 0};

	SDL_BindGPUVertexBuffers(curr_frame_state.render_pass, 0, &vb, 1);
	SDL_BindGPUIndexBuffer(curr_frame_state.render_pass, &ib, SDL_GPU_INDEXELEMENTSIZE_32BIT);

	SDL_GPUTexture *bound_tex = curr_renderer_resources.fallback_texture;
	SDL_GPUSampler *bound_sampler = curr_renderer_resources.default_sampler;

	if (mesh.desc.material_handle.valid())
	{
		WINDUP_Material &material = resources->get_material(mesh.desc.material_handle);
		if (material.desc.base_color_texture.valid())
		{
			WINDUP_Texture &texture = resources->get_texture(material.desc.base_color_texture);
			bound_tex = texture.texture;
			bound_sampler = texture.gpu_sampler;
		}
	}

	SDL_GPUTextureSamplerBinding tex_binding{};
	tex_binding.texture = bound_tex;
	tex_binding.sampler = bound_sampler;
	SDL_BindGPUFragmentSamplers(curr_frame_state.render_pass, 0, &tex_binding, 1);

	glm::mat4 combined = mvp.proj * mvp.view * mvp.model;
	SDL_PushGPUVertexUniformData(curr_frame_state.cmd_buffer, 0, &combined, sizeof(glm::mat4));
	SDL_DrawGPUIndexedPrimitives(curr_frame_state.render_pass, mesh.desc.indices.size(), 1, 0, 0, 0);
}

void WINDUP_Rendering::draw_gui()
{
	ImGui_ImplSDLGPU3_RenderDrawData(curr_frame_state.imgui_draw_data,
									 curr_frame_state.cmd_buffer,
									 curr_frame_state.render_pass);
}

//--------------------------------------------------------------------------------
// Bindings
//--------------------------------------------------------------------------------

bool WINDUP_Rendering::bind_window_to_gpu_device()
{
	WINDUP_GPUDevice *gpu_device = devices->get_gpu_device();
	WINDUP_Window *window = windowing->get_window();

	if (!SDL_ClaimWindowForGPUDevice(gpu_device, window))
	{
		WINDUP_Logger::error("SDL_ClaimWindowForGPUDevice failed: %s", SDL_GetError(), 0);
	}

	return true;
}

//--------------------------------------------------------------------------------
// Render pass
//--------------------------------------------------------------------------------

void WINDUP_Rendering::begin_base_render_pass()
{
	SDL_GPUColorTargetInfo color_target{};
	SDL_GPUDepthStencilTargetInfo depth_target{};

	if (engine_configs->enable_editor_mode)
	{
		WINDUP_SceneTarget& target = curr_renderer_resources.scene_target;
		if (!target.color || !target.depth)
		{
			curr_frame_state.render_pass = nullptr;
			return;
		}
		color_target.texture = target.color;
		depth_target.texture = target.depth;
	}
	else
	{
		// render straight to the screen
		color_target.texture = curr_frame_state.swapchain;
		depth_target.texture = curr_renderer_resources.depth_texture;  // window-sized
	}

	color_target.load_op   = SDL_GPU_LOADOP_CLEAR;
	color_target.store_op  = SDL_GPU_STOREOP_STORE;
	color_target.clear_color = {0.1f, 0.1f, 0.1f, 1.0f};

	depth_target.load_op   = SDL_GPU_LOADOP_CLEAR;
	depth_target.store_op  = SDL_GPU_STOREOP_DONT_CARE;
	depth_target.stencil_load_op  = SDL_GPU_LOADOP_CLEAR;
	depth_target.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
	depth_target.clear_depth = 1.0f;
	depth_target.cycle = true;

	curr_frame_state.render_pass = SDL_BeginGPURenderPass(
		curr_frame_state.cmd_buffer, &color_target, 1, &depth_target);
}

void WINDUP_Rendering::begin_gui_render_pass()
{
	SDL_GPUColorTargetInfo color_target{};
	color_target.texture = curr_frame_state.swapchain;
	color_target.load_op = SDL_GPU_LOADOP_CLEAR;
	color_target.store_op = SDL_GPU_STOREOP_STORE;
	color_target.clear_color = {0.0f, 0.0f, 0.0f, 1.0f};

	curr_frame_state.render_pass = SDL_BeginGPURenderPass(
		curr_frame_state.cmd_buffer, &color_target, 1, nullptr);
}

void WINDUP_Rendering::end_render_pass()
{
	SDL_EndGPURenderPass(curr_frame_state.render_pass);
	curr_frame_state.render_pass = nullptr;
}

