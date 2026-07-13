#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "tiny_gltf.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include "resources.hpp"

#include <fstream>


namespace
{
    struct ColorEntry { const char *name; ImGuiCol index; };

    constexpr std::array color_table{
        ColorEntry{"Text",                       ImGuiCol_Text},
        ColorEntry{"TextDisabled",               ImGuiCol_TextDisabled},
        ColorEntry{"WindowBg",                   ImGuiCol_WindowBg},
        ColorEntry{"ChildBg",                    ImGuiCol_ChildBg},
        ColorEntry{"PopupBg",                    ImGuiCol_PopupBg},
        ColorEntry{"Border",                     ImGuiCol_Border},
        ColorEntry{"BorderShadow",               ImGuiCol_BorderShadow},
        ColorEntry{"FrameBg",                    ImGuiCol_FrameBg},
        ColorEntry{"FrameBgHovered",             ImGuiCol_FrameBgHovered},
        ColorEntry{"FrameBgActive",              ImGuiCol_FrameBgActive},
        ColorEntry{"TitleBg",                    ImGuiCol_TitleBg},
        ColorEntry{"TitleBgActive",              ImGuiCol_TitleBgActive},
        ColorEntry{"TitleBgCollapsed",           ImGuiCol_TitleBgCollapsed},
        ColorEntry{"MenuBarBg",                  ImGuiCol_MenuBarBg},
        ColorEntry{"ScrollbarBg",                ImGuiCol_ScrollbarBg},
        ColorEntry{"ScrollbarGrab",              ImGuiCol_ScrollbarGrab},
        ColorEntry{"ScrollbarGrabHovered",       ImGuiCol_ScrollbarGrabHovered},
        ColorEntry{"ScrollbarGrabActive",        ImGuiCol_ScrollbarGrabActive},
        ColorEntry{"CheckMark",                  ImGuiCol_CheckMark},
        ColorEntry{"SliderGrab",                 ImGuiCol_SliderGrab},
        ColorEntry{"SliderGrabActive",           ImGuiCol_SliderGrabActive},
        ColorEntry{"Button",                     ImGuiCol_Button},
        ColorEntry{"ButtonHovered",              ImGuiCol_ButtonHovered},
        ColorEntry{"ButtonActive",               ImGuiCol_ButtonActive},
        ColorEntry{"Header",                     ImGuiCol_Header},
        ColorEntry{"HeaderHovered",              ImGuiCol_HeaderHovered},
        ColorEntry{"HeaderActive",               ImGuiCol_HeaderActive},
        ColorEntry{"Separator",                  ImGuiCol_Separator},
        ColorEntry{"SeparatorHovered",           ImGuiCol_SeparatorHovered},
        ColorEntry{"SeparatorActive",            ImGuiCol_SeparatorActive},
        ColorEntry{"ResizeGrip",                 ImGuiCol_ResizeGrip},
        ColorEntry{"ResizeGripHovered",          ImGuiCol_ResizeGripHovered},
        ColorEntry{"ResizeGripActive",           ImGuiCol_ResizeGripActive},
        ColorEntry{"InputTextCursor",            ImGuiCol_InputTextCursor},
        ColorEntry{"TabHovered",                 ImGuiCol_TabHovered},
        ColorEntry{"Tab",                        ImGuiCol_Tab},
        ColorEntry{"TabSelected",                ImGuiCol_TabSelected},
        ColorEntry{"TabSelectedOverline",        ImGuiCol_TabSelectedOverline},
        ColorEntry{"TabDimmed",                  ImGuiCol_TabDimmed},
        ColorEntry{"TabDimmedSelected",          ImGuiCol_TabDimmedSelected},
        ColorEntry{"TabDimmedSelectedOverline",  ImGuiCol_TabDimmedSelectedOverline},
        ColorEntry{"PlotLines",                  ImGuiCol_PlotLines},
        ColorEntry{"PlotLinesHovered",           ImGuiCol_PlotLinesHovered},
        ColorEntry{"PlotHistogram",              ImGuiCol_PlotHistogram},
        ColorEntry{"PlotHistogramHovered",       ImGuiCol_PlotHistogramHovered},
        ColorEntry{"TableHeaderBg",              ImGuiCol_TableHeaderBg},
        ColorEntry{"TableBorderStrong",          ImGuiCol_TableBorderStrong},
        ColorEntry{"TableBorderLight",           ImGuiCol_TableBorderLight},
        ColorEntry{"TableRowBg",                 ImGuiCol_TableRowBg},
        ColorEntry{"TableRowBgAlt",              ImGuiCol_TableRowBgAlt},
        ColorEntry{"TextLink",                   ImGuiCol_TextLink},
        ColorEntry{"TextSelectedBg",             ImGuiCol_TextSelectedBg},
        ColorEntry{"TreeLines",                  ImGuiCol_TreeLines},
        ColorEntry{"DragDropTarget",             ImGuiCol_DragDropTarget},
        ColorEntry{"DragDropTargetBg",           ImGuiCol_DragDropTargetBg},
        ColorEntry{"UnsavedMarker",              ImGuiCol_UnsavedMarker},
        ColorEntry{"NavCursor",                  ImGuiCol_NavCursor},
        ColorEntry{"NavWindowingHighlight",      ImGuiCol_NavWindowingHighlight},
        ColorEntry{"NavWindowingDimBg",          ImGuiCol_NavWindowingDimBg},
        ColorEntry{"ModalWindowDimBg",           ImGuiCol_ModalWindowDimBg},
    };
}

void WINDUP_Resources::init(WINDUP_EngineConfigs& arg_configs, WINDUP_Threading& arg_threading, WINDUP_Devices &arg_devices)
{
	devices = &arg_devices;
	status.f_is_init = true;

	WINDUP_Logger::task_result("Resources", "Initialization", status.f_is_init);
}

void WINDUP_Resources::deinit()
{
	status.f_is_deinit = true;

	WINDUP_Logger::task_result("Resources", "Deinitialization", status.f_is_deinit);
}

std::optional<YAML::Node> WINDUP_Resources::load_yaml(const std::string &path)
{
	try
	{
		YAML::Node result = YAML::LoadFile(path);
		return result;
	}
	catch (const YAML::BadFile &e)
	{
		WINDUP_Logger::error("yaml-cpp", "Loading YAML: could not open file " + path, 0);
		return std::nullopt;
	}
	catch (const YAML::ParserException &e)
	{
		WINDUP_Logger::error("yaml-cpp", "Loading YAML: " + std::string(e.what()), 0);
		return std::nullopt;
	}
}

bool WINDUP_Resources::load_style(const std::string &path, ImGuiStyle &style)
{
	auto yaml_opt = load_yaml(path);   // reuse the existing member
	if (!yaml_opt)
	{
		WINDUP_Logger::task_result("Load Style [" + path + "]", "FAILED - load error", 1);
		return false;
	}

	const YAML::Node colors = (*yaml_opt)["colors"];
	if (!colors || !colors.IsMap())
	{
		WINDUP_Logger::task_result("Load Style [" + path + "]", "FAILED - no 'colors' map", 1);
		return false;
	}

	for (const auto &entry : color_table)
	{
		const YAML::Node c = colors[entry.name];
		if (!c || !c.IsSequence() || c.size() < 4)
			continue;  // missing/malformed — keep the style's existing default

		ImVec4 &dst = style.Colors[entry.index];
		dst.x = c[0].as<float>(dst.x);
		dst.y = c[1].as<float>(dst.y);
		dst.z = c[2].as<float>(dst.z);
		dst.w = c[3].as<float>(dst.w);
	}

	WINDUP_Logger::task_result("Load Style [" + path + "]", "SUCCESS", 1);
	return true;
}

bool WINDUP_Resources::save_style(const std::string &path, const ImGuiStyle &style)
{
	YAML::Node root;
	YAML::Node colors;

	for (const auto &entry : color_table)
	{
		const ImVec4 &c = style.Colors[entry.index];
		YAML::Node rgba;
		rgba.SetStyle(YAML::EmitterStyle::Flow);   // [r, g, b, a] on one line
		rgba.push_back(c.x);
		rgba.push_back(c.y);
		rgba.push_back(c.z);
		rgba.push_back(c.w);
		colors[entry.name] = rgba;
	}
	root["colors"] = colors;

	std::ofstream file(path);
	if (!file.is_open())
	{
		WINDUP_Logger::task_result("Save Style [" + path + "]", "FAILED - cannot open file", 0);
		return false;
	}
	file << root;

	WINDUP_Logger::task_result("Save Style [" + path + "]", file.good() ? "SUCCESS" : "FAILURE", 0);
	return file.good();
}

WINDUP_ShaderHandle WINDUP_Resources::instantiate_shader(const WINDUP_ShaderDesc &shader_desc)
{
	WINDUP_GPUDevice *gpu_device = devices->get_gpu_device();

	WINDUP_Shader shader{.desc = shader_desc};

	// Vertex shader
	size_t vertex_shader_code_size = 0;
	void *vert_shader_code = SDL_LoadFile(shader_desc.vert_path.c_str(), &vertex_shader_code_size);
	if (!vert_shader_code)
	{
		WINDUP_Logger::task_result("Load Vertex Shader [" + shader_desc.vert_path + "]", "FAILED - file not found", 0);
		return {};
	}

	SDL_GPUShaderCreateInfo vertex_shader_info{};
	vertex_shader_info.code = static_cast<const Uint8 *>(vert_shader_code);
	vertex_shader_info.code_size = vertex_shader_code_size;
	vertex_shader_info.entrypoint = "main";
	vertex_shader_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
	vertex_shader_info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
	vertex_shader_info.num_uniform_buffers = 1;
	vertex_shader_info.num_samplers = 0;

	shader.vert = SDL_CreateGPUShader(devices->get_gpu_device(), &vertex_shader_info);
	SDL_free(vert_shader_code);

	if (!shader.vert)
	{
		WINDUP_Logger::task_result("Create Vertex Shader [" + shader_desc.vert_path + "]",
		                       std::string("FAILED - ") + SDL_GetError(), 0);
	}

	size_t fragment_shader_code_size = 0;
	void *code = SDL_LoadFile(shader_desc.frag_path.c_str(), &fragment_shader_code_size);
	if (!code)
	{
		WINDUP_Logger::task_result("Load Fragment Shader [" + shader_desc.frag_path + "]", "FAILED - file not found", 0);
		return {};
	}

	SDL_GPUShaderCreateInfo fragment_shader_info{};
	fragment_shader_info.code = static_cast<const Uint8 *>(code);
	fragment_shader_info.code_size = fragment_shader_code_size;
	fragment_shader_info.entrypoint = "main";
	fragment_shader_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
	fragment_shader_info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	fragment_shader_info.num_uniform_buffers = 0;
	fragment_shader_info.num_samplers = 1;

	shader.frag = SDL_CreateGPUShader(devices->get_gpu_device(), &fragment_shader_info);
	SDL_free(code);

	if (!shader.frag)
	{
		WINDUP_Logger::task_result("Create Fragment Shader [" + shader_desc.frag_path + "]",
							   std::string("FAILED - ") + SDL_GetError(), 0);
		return {};
	}

	if (!shader.vert || !shader.frag)
	{
		if (shader.vert) SDL_ReleaseGPUShader(gpu_device, shader.vert);
		if (shader.frag) SDL_ReleaseGPUShader(gpu_device, shader.frag);
		WINDUP_Logger::task_result("Load Shader [" + shader_desc.name + "]", "FAILED", 0);
		return {}; // invalid handle - callers must check valid()
	}

	WINDUP_ShaderHandle handle = shaders_cache.cache_object(shader_desc.name, shader);

	WINDUP_Logger::task_result("Load Shader [" + shader_desc.name + "]", "SUCCESS", 1);
	return handle;
}

WINDUP_TextureHandle WINDUP_Resources::instantiate_texture(WINDUP_TextureDesc &texture_desc)
{
	if (texture_desc.image_data.empty() && !texture_desc.file_path.empty())
	{
		std::string full_path = base_path + texture_desc.file_path;

		int w = 0, h = 0, channels = 0;
		unsigned char *pixels = stbi_load(full_path.c_str(), &w, &h, &channels, 4); // RGBA
		if (!pixels)
		{
			WINDUP_Logger::error("Resources",
			                 std::string("stbi_load failed: ") + full_path + " — " + stbi_failure_reason(), 0);
			return {};
		}

		texture_desc.width = w;
		texture_desc.height = h;
		texture_desc.image_data.assign(pixels, pixels + (size_t) w * h * 4);
		stbi_image_free(pixels);
	}

	WINDUP_Logger::task_result("Load Texture [" + texture_desc.name
	                       + "] of size " + std::to_string(texture_desc.image_data.size()) + " bytes ",
	                       "SUCCESS", 1);

	WINDUP_Texture texture;
	texture.desc = texture_desc;

	WINDUP_TextureHandle handle = textures_cache.cache_object(texture_desc.name, texture);
	textures_cache.get_by_handle(handle).handle = handle;
	return handle;
}

WINDUP_MaterialHandle WINDUP_Resources::instantiate_material(const WINDUP_MaterialDesc &material_desc)
{
	WINDUP_Logger::task_result("Load Material [" + material_desc.name + "] ", "SUCCESS", 1);

	WINDUP_Material material;
	material.desc = material_desc; // desc now contains base_color_texture handle + base_color

	WINDUP_MaterialHandle handle = materials_cache.cache_object(material_desc.name, material);
	materials_cache.get_by_handle(handle).handle = handle;

	return handle;
}

WINDUP_MeshHandle WINDUP_Resources::instantiate_mesh(const WINDUP_MeshDesc &mesh_desc)
{
	WINDUP_Mesh mesh{.desc = mesh_desc};
	WINDUP_MeshHandle mesh_handle = meshes_cache.cache_object(mesh_desc.name, mesh);
	WINDUP_Mesh &cached_mesh = meshes_cache.get_by_handle(mesh_handle);
	cached_mesh.handle = mesh_handle;
	return mesh_handle;
}

WINDUP_ModelHandle WINDUP_Resources::instantiate_model(WINDUP_ModelDesc &model_desc)
{
	tinygltf::TinyGLTF tgltf_loader;
	tinygltf::Model tgltf_model;
	std::string tgltf_err;
	std::string tgltf_warn;

	bool is_binary_loaded = tgltf_loader.LoadBinaryFromFile(
		&tgltf_model, &tgltf_err, &tgltf_warn, model_desc.file_path);

	if (configs.debug)
	{
		if (!tgltf_warn.empty()) WINDUP_Logger::warning("tinygltf", "Loading model: " + tgltf_warn, 1);
		if (!tgltf_err.empty()) WINDUP_Logger::error("tinygltf", "Loading model: " + tgltf_err, 1);
	}

	if (!is_binary_loaded) return {};

	WINDUP_Model model;
	model.desc.name = model_desc.name;
	model.desc.file_path = model_desc.file_path;

	std::unordered_map<int, WINDUP_TextureHandle> source_to_texture;

	std::function < void(int, glm::mat4) > process_node = [&](int node_index, glm::mat4 parent_matrix)
	{
		if (node_index < 0 || node_index >= (int) tgltf_model.nodes.size()) return;
		const auto &node = tgltf_model.nodes[node_index];

		glm::mat4 node_matrix = glm::mat4(1.0f);

		if (node.matrix.size() == 16)
		{
			node_matrix = glm::mat4(
				node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3],
				node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7],
				node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11],
				node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]
			);
		} else
		{
			glm::vec3 translation = node.translation.size() == 3
				                        ? glm::vec3(node.translation[0], node.translation[1], node.translation[2])
				                        : glm::vec3(0.0f);
			glm::quat rotation = node.rotation.size() == 4
				                     ? glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2])
				                     : glm::quat(1, 0, 0, 0);
			glm::vec3 scale = node.scale.size() == 3
				                  ? glm::vec3(node.scale[0], node.scale[1], node.scale[2])
				                  : glm::vec3(1.0f);

			node_matrix = glm::translate(glm::mat4(1.0f), translation)
			              * glm::mat4_cast(rotation)
			              * glm::scale(glm::mat4(1.0f), scale);
		}

		glm::mat4 world_matrix = parent_matrix * node_matrix;
		glm::mat4 normal_matrix = glm::transpose(glm::inverse(world_matrix));

		if (node.mesh >= 0 && node.mesh < (int) tgltf_model.meshes.size())
		{
			const auto &tgltf_mesh = tgltf_model.meshes[node.mesh];

			for (const auto &primitive: tgltf_mesh.primitives)
			{
				WINDUP_MeshDesc mesh_desc;
				mesh_desc.name = node.name.empty()
					                      ? ("mesh_" + std::to_string(node_index))
					                      : node.name;

				auto get_attribute_data = [&](const char *name) -> const float *
				{
					auto it = primitive.attributes.find(name);
					if (it == primitive.attributes.end()) return nullptr;

					const auto &accessor = tgltf_model.accessors[it->second];
					const auto &view = tgltf_model.bufferViews[accessor.bufferView];
					const auto *data = tgltf_model.buffers[view.buffer].data.data()
					                   + view.byteOffset + accessor.byteOffset;
					return reinterpret_cast<const float *>(data);
				};

				auto pos_it = primitive.attributes.find("POSITION");
				if (pos_it == primitive.attributes.end())
				{
					if (configs.debug)
						WINDUP_Logger::warning("Resources", "Primitive missing POSITION, skipping", 1);
					continue;
				}
				const auto &pos_accessor = tgltf_model.accessors[pos_it->second];
				const float *positions = get_attribute_data("POSITION");
				const float *normals = get_attribute_data("NORMAL");
				const float *uvs = get_attribute_data("TEXCOORD_0");

				if (!positions)
				{
					if (configs.debug)
						WINDUP_Logger::warning("Resources", "Primitive POSITION data null, skipping", 1);
					continue;
				}

				if (primitive.material >= 0 && primitive.material < (int) tgltf_model.materials.size())
				{
					const auto &mat = tgltf_model.materials[primitive.material];

					int tex_index = mat.pbrMetallicRoughness.baseColorTexture.index;

					if (tex_index < 0 || tex_index >= (int) tgltf_model.textures.size())
					{
						if (configs.debug)
							WINDUP_Logger::warning("Resources", "Material '" + mat.name + "' has no base color texture", 1);
					} else
					{
						const auto &texture = tgltf_model.textures[tex_index];

						if (texture.source < 0 || texture.source >= (int) tgltf_model.images.size())
						{
							if (configs.debug)
								WINDUP_Logger::warning("Resources", "Material '" + mat.name + "' bad texture source", 1);
						} else
						{
							WINDUP_TextureHandle texture_handle;

							if (auto it = source_to_texture.find(texture.source);
								it != source_to_texture.end())
							{
								texture_handle = it->second;
							} else
							{
								const auto &image = tgltf_model.images[texture.source];

								if (configs.debug)
									WINDUP_Logger::info("Resources",
									                "Material '" + mat.name
									                + "' tex_index=" + std::to_string(tex_index)
									                + " texture.source=" + std::to_string(texture.source)
									                + " image=" + std::to_string(image.width) + "x" + std::to_string(
										                image.height)
									                + " bytes=" + std::to_string(image.image.size()), 1);

								if (image.width <= 0 || image.height <= 0 || image.image.empty())
								{
									if (configs.debug)
										WINDUP_Logger::warning("Resources",
										                   "Material '" + mat.name +
										                   "' image failed to decode, skipping", 1);
								} else
								{
									WINDUP_TextureDesc texture_desc{
										.name = mat.name + "_albedo",
										.file_path = model_desc.file_path,
										.width = image.width,
										.height = image.height,
										.image_data = image.image
									};
									texture_handle = instantiate_texture(texture_desc);
									source_to_texture[texture.source] = texture_handle;
								}
							}

							if (texture_handle.valid())
							{
								WINDUP_MaterialDesc material_desc{
									.name = mat.name,
									.file_path = model_desc.file_path,
									.base_color_texture = texture_handle,
									.base_color = glm::vec4(
										mat.pbrMetallicRoughness.baseColorFactor[0],
										mat.pbrMetallicRoughness.baseColorFactor[1],
										mat.pbrMetallicRoughness.baseColorFactor[2],
										mat.pbrMetallicRoughness.baseColorFactor[3])
								};

								WINDUP_MaterialHandle material_handle = instantiate_material(material_desc);

								if (material_handle.valid())
								{
									mesh_desc.material_handle = material_handle;
								} else
								{
									SDL_Log("ERROR: Material instantiation failed for %s", mat.name.c_str());
								}
							}
						}
					}
				}

				mesh_desc.vertices.resize(pos_accessor.count);

				for (size_t i = 0; i < pos_accessor.count; ++i)
				{
					glm::vec3 local_pos(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
					glm::vec4 world_pos = world_matrix * glm::vec4(local_pos, 1.0f);
					mesh_desc.vertices[i].position = glm::vec3(world_pos);

					if (normals)
					{
						glm::vec3 local_norm(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
						glm::vec4 world_norm = normal_matrix * glm::vec4(local_norm, 0.0f);
						mesh_desc.vertices[i].normal = glm::normalize(glm::vec3(world_norm));
					} else
					{
						mesh_desc.vertices[i].normal = glm::vec3(0.0f, 1.0f, 0.0f);
					}

					mesh_desc.vertices[i].uv = uvs
						                           ? glm::vec2(uvs[i * 2], uvs[i * 2 + 1])
						                           : glm::vec2(0.0f, 0.0f);
				}

				if (primitive.indices >= 0)
				{
					const auto &idx_accessor = tgltf_model.accessors[primitive.indices];
					const auto &idx_view = tgltf_model.bufferViews[idx_accessor.bufferView];
					const uint8_t *idx_data = tgltf_model.buffers[idx_view.buffer].data.data()
					                          + idx_view.byteOffset + idx_accessor.byteOffset;

					mesh_desc.indices.resize(idx_accessor.count);

					switch (idx_accessor.componentType)
					{
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
							for (size_t i = 0; i < idx_accessor.count; ++i)
								mesh_desc.indices[i] = reinterpret_cast<const uint8_t *>(idx_data)[i];
							break;

						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
							for (size_t i = 0; i < idx_accessor.count; ++i)
								mesh_desc.indices[i] = reinterpret_cast<const uint16_t *>(idx_data)[i];
							break;

						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
							for (size_t i = 0; i < idx_accessor.count; ++i)
								mesh_desc.indices[i] = reinterpret_cast<const uint32_t *>(idx_data)[i];
							break;

						default:
							WINDUP_Logger::error("Resources",
							                 "Unsupported index component type: "
							                 + std::to_string(idx_accessor.componentType), 0);
					}
				}

				WINDUP_MeshHandle mesh_handle = instantiate_mesh(mesh_desc);
				model.desc.mesh_handles.push_back(mesh_handle);
			}
		}

		for (int child_index: node.children)
			process_node(child_index, world_matrix);
	};

	if (tgltf_model.scenes.empty())
	{
		WINDUP_Logger::error("Resources", "GLB has no scenes: " + model_desc.name, 0);
		return {};
	}

	const auto &scene = tgltf_model.scenes[
		tgltf_model.defaultScene >= 0 ? tgltf_model.defaultScene : 0];
	for (int root_node: scene.nodes)
		process_node(root_node, glm::mat4(1.0f));

	WINDUP_ModelHandle model_handle = models_cache.cache_object(model.desc.name, model);
	models_cache.get_by_handle(model_handle).handle = model_handle;

	WINDUP_Logger::task_result("Instantiate Model [" + model_desc.name + "]", "SUCCESS", 1);
	return model_handle;
}

