#include "windup.hpp"

//--------------------------------------------------------------------------------
// Init / Deinit
//--------------------------------------------------------------------------------

void WINDUP_ENGINE::init()
{
	status.f_is_running = true;

	windup_ctx.engine_status = &status;
	windup_ctx.engine_configs = &engine_configs;
	init_sdl_video();

	base_path = SDL_GetBasePath();

	// CORE SYSTEMS
	WINDUP_Logger::init();

	// MODULES
	init_modules();
	windup_ctx.modules = create_modules_ctx();

	// TOOLS
	commands.init(windup_ctx);
	windup_ctx.console = &commands;

	register_engine_cmds();

	init_editor_gui();

	status.f_is_init = true;

	WINDUP_Logger::task_result("Engine", "Initialization", status.f_is_init);
}

void WINDUP_ENGINE::deinit()
{
	status.f_is_closing = true;

	modules.renderer->deinit();

	status.f_is_running = false;
	status.f_is_deinit = true;
	WINDUP_Logger::task_result("Engine", "Deitialization", status.f_is_deinit);
}

//--------------------------------------------------------------------------------
// Run/quit
//--------------------------------------------------------------------------------

bool WINDUP_ENGINE::run()
{
	init();

	if (loaded_app)
	{
		loaded_app->on_run(windup_ctx);
	}

	while (status.f_is_running)
	{
		if (loaded_app)
		{
			loaded_app->on_update();
		}
	}

	return true;
}

bool WINDUP_ENGINE::quit()
{
	status.f_is_running = false;
	loaded_app->status.f_is_running = false;

	return true;
}

//--------------------------------------------------------------------------------
// App operations
//--------------------------------------------------------------------------------

bool WINDUP_ENGINE::load_app(WINDUP_App *app)
{
	loaded_app = app;

	return true;
}

//--------------------------------------------------------------------------------
// Third-party
//--------------------------------------------------------------------------------

bool WINDUP_ENGINE::init_sdl_video()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_Log("SDL_Init failed: %s", SDL_GetError());
		SDL_assert(false);
	}

	return true;
}

//--------------------------------------------------------------------------------
// Modules
//--------------------------------------------------------------------------------

bool WINDUP_ENGINE::init_modules()
{
	modules.time = init_module<WINDUP_Time>(*windup_ctx.engine_configs);
	modules.threading = init_module<WINDUP_Threading>(*windup_ctx.engine_configs);
	modules.profiler = init_module<WINDUP_Profiler>(*windup_ctx.engine_configs);
	modules.devices = init_module<WINDUP_Devices>(*windup_ctx.engine_configs, *modules.threading);
	modules.resources = init_module<WINDUP_Resources>(*windup_ctx.engine_configs, *modules.threading, *modules.devices);
	modules.windowing = init_module<WINDUP_Windowing>(*windup_ctx.engine_configs, *modules.threading);
	modules.ecs = init_module<WINDUP_ECS>(*windup_ctx.engine_configs, *modules.threading);
	modules.io = init_module<WINDUP_UserIO>(*windup_ctx.engine_configs, *modules.threading);
	modules.renderer = init_module<WINDUP_Rendering>(*windup_ctx.engine_configs, *modules.threading, *modules.devices, *modules.resources, *modules.windowing);
	modules.levels = init_module<WINDUP_Levels>(*windup_ctx.engine_configs, *modules.threading, *modules.resources, *modules.ecs, *modules.renderer);

	return true;
}

WINDUP_ModulesContext WINDUP_ENGINE::create_modules_ctx()
{
	WINDUP_ModulesContext modules_ctx{
									.time = modules.time.get(),
									.threading = modules.threading.get(),
									.profiler = modules.profiler.get(),
									.devices = modules.devices.get(),
									.resources = modules.resources.get(),
									.windowing = modules.windowing.get(),
									.ecs = modules.ecs.get(),
									.io = modules.io.get(),
									.renderer = modules.renderer.get(),
									.level_manager = modules.levels.get()
	};

	return modules_ctx;
}

//--------------------------------------------------------------------------------
// Cmds
//--------------------------------------------------------------------------------

bool WINDUP_ENGINE::register_engine_cmds()
{
	commands.register_immediate_cmd("quit_engine", [this](auto& args){
		WINDUP_Logger::cmd_result("Engine", "Quitting", 1);
		quit();
		return true;
	});

	return true;
}

//--------------------------------------------------------------------------------
// Editor
//--------------------------------------------------------------------------------

bool WINDUP_ENGINE::init_editor_gui()
{
    WINDUP_Resources *resources = modules.resources.get();
    WINDUP_Rendering *renderer  = modules.renderer.get();

    // ── One-time setup ───────────────────────────────────────────────────────
    WINDUP_GUIDesc editor_gui_desc;
    editor_gui_desc.name = "editor_gui_style";
    ImGui::StyleColorsDark(&editor_gui_desc.style);                    // baseline
    resources->load_style(base_path + "configs/editor_gui_style.yaml", editor_gui_desc.style);
    renderer->create_editor_gui(editor_gui_desc);

    WINDUP_TextureDesc editor_bg_desc;
    editor_bg_desc.name      = "editor_bg_texture";
    editor_bg_desc.file_path = "assets/textures/" + renderer->renderer_configs.editor_background;

    WINDUP_TextureHandle editor_bg_handle = resources->instantiate_texture(editor_bg_desc);

    // upload to the GPU (no-op-safe if instantiate failed — handle is invalid)
    renderer->upload_texture(editor_bg_handle);

    // resolve the GPU texture ONCE here — not per-frame in the lambda
    SDL_GPUTexture *bg_tex = nullptr;
    if (editor_bg_handle.valid())
    {
        bg_tex = resources->get_texture(editor_bg_handle).texture;
        WINDUP_Logger::info("Annihilator", "Editor background texture: " + editor_bg_desc.name, 1);
    }
    else
    {
        WINDUP_Logger::warning("Annihilator", "Editor background texture not available", 0);
    }

    // ── Per-frame editor GUI ─────────────────────────────────────────────────
    renderer->submit_editor_gui_cmds([this, bg_tex, renderer]() {

        // ── Background (tiled, behind all editor windows) ────────────────────
        if (bg_tex)
        {
            const float tile = 128.0f;
            ImGuiViewport *vp = ImGui::GetMainViewport();
            ImDrawList *bg = ImGui::GetBackgroundDrawList();
            for (float y = vp->Pos.y; y < vp->Pos.y + vp->Size.y; y += tile)
                for (float x = vp->Pos.x; x < vp->Pos.x + vp->Size.x; x += tile)
                    bg->AddImage((ImTextureID)bg_tex,
                                 ImVec2(x, y), ImVec2(x + tile, y + tile));
        }

    	// ── Menu Bar ─────────────────────────────────────────────────────────
  //   	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
		// ImGui::SetNextWindowSize(ImVec2(1260, 20), ImGuiCond_Always);
    	if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Level"))  { /* ... */ }
				if (ImGui::MenuItem("Open Level")) { /* ... */ }
				if (ImGui::MenuItem("Save Level")) { /* ... */ }
				ImGui::Separator();
				if (ImGui::MenuItem("Quit"))       { loaded_app->on_quit(); }
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Settings"))
			{
				if (ImGui::MenuItem("Reset Layout")) { ImGui::LoadIniSettingsFromMemory("", 0); }
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Editor Mode", nullptr, true)) { /* toggle */ }
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}


    	// ── Play controls ────────────────────────────────────────────────────
    	ImGui::SetNextWindowPos(ImVec2(800, 40), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(832, 60), ImGuiCond_Always);
		ImGui::Begin("Toolbar");

			if (ImGui::Button("Play"))
			{
				;   // false = game/scene mode
			}

		ImGui::End();

    	// ── Console ──────────────────────────────────────────────────────────
    	ImGui::SetNextWindowPos(ImVec2(50, 860), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(610, 200), ImGuiCond_Always);
    	ImGui::Begin("Developer Console");

				const float input_h = ImGui::GetFrameHeightWithSpacing();
				ImGui::BeginChild("console_log", ImVec2(0, -input_h), ImGuiChildFlags_Borders,
								  ImGuiWindowFlags_HorizontalScrollbar);

				for (const auto& e : WINDUP_Logger::get_entries())
				{
					ImVec4 col(0.85f, 0.85f, 0.85f, 1.0f);   // default

					if (e.level == WINDUP_LogLevel::Info)
					{

					}

					if (e.level == WINDUP_LogLevel::Warning)
					{
						col = ImVec4(0.96f, 0.72f, 0.36f, 1.0f);
					}

					if (e.level == WINDUP_LogLevel::Error)
					{
						col = ImVec4(0.74f, 0.14f, 0.22f, 1.0f);
					}

					if (e.level == WINDUP_LogLevel::CmdResult)
					{
						if (e.message.find("SUCCESS") != std::string::npos)
						{
							col = ImVec4(0.34f, 0.60f, 0.31f, 1.0f);
						}
						else if (e.message.find("FAILURE") != std::string::npos)
						{
							col = ImVec4(0.74f, 0.14f, 0.22f, 1.0f);
						}
					}

					if (e.level == WINDUP_LogLevel::TaskResult)
					{
						if (e.message.find("SUCCESS") != std::string::npos)
						{
							col = ImVec4(0.34f, 0.60f, 0.31f, 1.0f);
						}
						else if (e.message.find("FAILURE") != std::string::npos)
						{
							col = ImVec4(0.74f, 0.14f, 0.22f, 1.0f);
						}
					}

					std::string_view lvl = WINDUP_Logger::to_string(e.level);
					ImGui::TextColored(col, "[%.*s] %s: %s",
														   (int)lvl.size(), lvl.data(),
														   e.category.c_str(),
														   e.message.c_str());
				}

				// auto-scroll only when already pinned to the bottom
				if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
					ImGui::SetScrollHereY(1.0f);

				ImGui::EndChild();

				// input line
				static char input_buf[256] = "";
				ImGui::SetNextItemWidth(-FLT_MIN);

				if (ImGui::InputText("##console_input", input_buf, sizeof(input_buf),
									 ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (input_buf[0] != '\0')
					{
						commands.execute(input_buf);
						input_buf[0] = '\0';
					}
					ImGui::SetKeyboardFocusHere(-1);   // keep focus on the input after submit
				}

		ImGui::End();

        // ── Performance ──────────────────────────────────────────────────────
    	ImGui::SetNextWindowPos(ImVec2(50, 450), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Always);
    	ImGui::Begin("Performance");
        if (ImGui::CollapsingHeader("Framerate Monitoring"))
        {
            ImGui::PlotLines("##fps",
                             windup_ctx.modules.profiler->fps_data(),
                             windup_ctx.modules.profiler->fps_count(),
                             0, "", 0.0f, 60.0f, ImVec2(0, 80));
            ImGui::Text("FPS: %.1f", windup_ctx.modules.profiler->fps_average());
        }
        ImGui::End();

        // ── Configs ──────────────────────────────────────────────────────────
    	ImGui::SetNextWindowPos(ImVec2(50, 40), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Always);

    	ImGui::Begin("Configs");
        if (ImGui::CollapsingHeader("Logging"))
        {
            if (ImGui::Checkbox("Print Cmd Results", &WINDUP_Logger::configs.print_level_cmd_result))
                WINDUP_Logger::info("Config", std::string("Print Cmd Results: ") +
                                (WINDUP_Logger::configs.print_level_cmd_result ? "true" : "false"), 1);

            if (ImGui::Checkbox("Print Info", &WINDUP_Logger::configs.print_level_info))
                WINDUP_Logger::info("Config", std::string("Print Info: ") +
                                (WINDUP_Logger::configs.print_level_info ? "true" : "false"), 1);

            if (ImGui::Checkbox("Print Task Results", &WINDUP_Logger::configs.print_level_task_result))
                WINDUP_Logger::info("Config", std::string("Print Task Results: ") +
                                (WINDUP_Logger::configs.print_level_task_result ? "true" : "false"), 1);

            if (ImGui::Checkbox("Print Warnings", &WINDUP_Logger::configs.print_level_warning))
                WINDUP_Logger::info("Config", std::string("Print Warnings: ") +
                                (WINDUP_Logger::configs.print_level_warning ? "true" : "false"), 1);

            if (ImGui::Checkbox("Print Errors", &WINDUP_Logger::configs.print_level_error))
                WINDUP_Logger::info("Config", std::string("Print Errors: ") +
                                (WINDUP_Logger::configs.print_level_error ? "true" : "false"), 1);
        }
        ImGui::End();

        // ── Scene Viewer ─────────────────────────────────────────────────────
    	ImGui::SetNextWindowPos(ImVec2(800, 110), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(832, 624), ImGuiCond_Always);
        ImGui::Begin("Scene Viewer");

        ImVec2 img_pos = ImGui::GetCursorScreenPos();
        ImVec2 avail   = ImGui::GetContentRegionAvail();

        renderer->create_scene_target((uint32_t)avail.x, (uint32_t)avail.y);

        SDL_GPUTexture *scene_tex = renderer->get_scene_texture();
        if (scene_tex && avail.x > 0 && avail.y > 0)
        {
            ImGui::Image((ImTextureID)scene_tex, avail);
        }

        ImGui::End();
    });

    return true;
}

