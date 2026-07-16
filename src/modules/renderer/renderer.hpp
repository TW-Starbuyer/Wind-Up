#pragma once

// STL INCLUDES
#include <unordered_map>
#include <string>
#include <functional>

// THIRD-PARTY INCLUDES
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

// CORE INCLUDES
#include "../logger/logger.hpp"

// MODULE INCLUDES
#include "../../modules/devices/devices.hpp"
#include "../../modules/resources/resources.hpp"
#include "../../modules/windowing/windowing.hpp"

// COMMON INCLUDES
#include "../../common/module.hpp"
#include "../../common/configs.hpp"
#include "../../common/components.hpp"
#include "../../common/math.hpp"
#include "../../common/shader.hpp"
#include "../../common/cache.hpp"
#include "../../common/pipeline.hpp"
#include "../../common/model.hpp"
#include "../../common/material.hpp"
#include "../../common/mvp.hpp"
#include "../../common/render_data.hpp"
#include "../../common/gui.hpp"

//----------------------------------------------------------------------------------------------

class WINDUP_Rendering : public WINDUP_Module
{
	public:
		WINDUP_RendererConfigs renderer_configs;

		WINDUP_FrameState curr_frame_state;
		WINDUP_RendererResources curr_renderer_resources;

		WINDUP_Rendering() = default;
		~WINDUP_Rendering() = default;

		void init(WINDUP_EngineConfigs& arg_global_configs,
												WINDUP_Threading& arg_threading,
												WINDUP_Devices& arg_devices,
												WINDUP_Resources& arg_resources,
												WINDUP_Windowing& arg_windowing);
		void deinit();

		WINDUP_PipelineHandle create_pipeline(WINDUP_PipelineDesc desc);
		WINDUP_PipelineHandle get_pipeline(const std::string &name);
		void use_pipeline(WINDUP_PipelineHandle handle);
		void use_pipeline(const std::string &name);
		bool destroy_pipeline(WINDUP_PipelineHandle handle);

		bool begin_frame();
		void end_frame();

		void create_scene_target(uint32_t w, uint32_t h);
		SDL_GPUTexture *get_scene_texture() { return curr_renderer_resources.scene_target.color; }
		void request_scene_size(uint32_t w, uint32_t h) { create_scene_target(w, h); }

		void upload_mesh(WINDUP_MeshHandle mesh_handle);

		void upload_model(WINDUP_ModelHandle model_handle);

		void upload_texture(WINDUP_TextureHandle texture_handle);
		void create_depth_texture();
		void create_fallback_texture();

		void upload_material(WINDUP_MaterialHandle material_handle);

		void create_sampler();

		void prepare_gui();

		bool submit_draw_call(WINDUP_DrawCall draw_call);
		bool execute_render_queue();

		void draw_base(const WINDUP_Mesh &mesh, const WINDUP_MVP &mvp);
		void draw_gui();

		bool bind_window_to_gpu_device();

		void begin_base_render_pass();
		void begin_gui_render_pass();
		void end_render_pass();

		void create_app_gui(WINDUP_GUIDesc arg_gui_desc);
		void create_editor_gui(WINDUP_GUIDesc layer_desc);
		void submit_app_gui_cmds(std::function<void()> cmds) { app_gui_cmds = std::move(cmds); }
		void submit_editor_gui_cmds(std::function<void()> cmds) { editor_gui_cmds = std::move(cmds); }
		bool load_editor_background(const std::string& filename);

	private:
		WINDUP_EngineConfigs* engine_configs = nullptr;
		WINDUP_Devices* devices = nullptr;
		WINDUP_Resources* resources = nullptr;
		WINDUP_Windowing* windowing = nullptr;

		WINDUP_GUIDesc app_ui_desc;
		WINDUP_GUIDesc editor_ui_desc;

		WINDUP_Cache<WINDUP_Pipeline, WINDUP_PipelineHandle> pipelines_cache;
		std::vector<WINDUP_DrawCall> render_queue;

		bool init_gui();
		void apply_theme(ImGuiStyle &style);
		void execute_app_gui_cmds();
		void execute_editor_gui_cmds();
		std::function<void()> app_gui_cmds;
		std::function<void()> editor_gui_cmds;
};

