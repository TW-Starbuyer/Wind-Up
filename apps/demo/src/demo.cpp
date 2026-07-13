#include "demo.hpp"

#include <ranges>

//--------------------------------------------------------------------------------
// Run/quit
//--------------------------------------------------------------------------------

void Demo::on_run(WINDUP_EngineContext& elements_refs)
{
    base_path = SDL_GetBasePath();

    windup = &elements_refs;

    register_app_cmds();

    init_app_gui();

	WINDUP_Levels* level_manager = windup->modules.level_manager;
    level_manager->load_level("room");

    set_editor_mode(windup->engine_configs->enable_editor_mode);

    windup->engine_status->f_is_running = true;
}

void Demo::on_quit()
{
    status.f_is_running = false;
    windup->engine_status->f_is_running = false;
}

//--------------------------------------------------------------------------------
// On update
//--------------------------------------------------------------------------------

void Demo::on_update()
{
    on_windowing_update();
    on_devices_update();
    on_resources_update();
    on_time_update();
    on_ecs_update();
    on_io_update();
    on_renderer_update();
    on_level_manager_update();
    on_console_update();
    on_logger_update();
}

void Demo::on_windowing_update()
{
    WINDUP_Windowing* windowing = windup->modules.windowing;

    windowing->configure_window();
}

void Demo::on_devices_update()
{
    WINDUP_Devices* devices = windup->modules.devices;
}

void Demo::on_resources_update()
{
    WINDUP_Resources* resources = windup->modules.resources;
}

void Demo::on_time_update()
{
    WINDUP_Time* time = windup->modules.time;

    time->tick();
}

void Demo::on_ecs_update()
{
    WINDUP_Resources* resources = windup->modules.resources;
    WINDUP_Windowing* windowing = windup->modules.windowing;
    WINDUP_Time* time = windup->modules.time;
    WINDUP_ECS* ecs = windup->modules.ecs;
    WINDUP_Rendering* renderer = windup->modules.renderer;
    flecs::world& registry = ecs->get_registry();

    float dt = time->get_delta_time();

    glm::mat4 view_matrix = glm::mat4(1.0f);
    glm::mat4 proj_matrix = glm::mat4(1.0f);

    auto [window_w, window_h] = windowing->get_window_dimensions();
    float aspect = (float) window_w / (float) window_h;

    registry.each([&](WINDUP_CameraCmpt& cam, WINDUP_TransformCmpt& trn)
    {
        if (!cam.active) return;

        trn.rotation.z = cam.yaw + cam.yaw_offset;

        glm::vec3 flat_fwd = glm::normalize(glm::vec3(cam.direction.x, cam.direction.y, 0.0f));

        glm::vec3 eye = trn.position
                      + glm::vec3(0, 0, cam.eye_height)
                      + flat_fwd * cam.eye_offset;

        view_matrix = glm::lookAt(
            eye,
            eye + cam.direction,
            glm::vec3(0,0,1)
        );

        proj_matrix = glm::perspective(glm::radians(cam.fov_degrees), aspect, cam.near_plane, cam.far_plane);
        proj_matrix[1][1] *= -1;
    });

    registry.each([&](WINDUP_ControllableCmpt& ctrl, WINDUP_VelocityCmpt& vel, WINDUP_TransformCmpt& trn, WINDUP_CameraCmpt& cam)
    {
        if (!ctrl.active) return;
        if (!cam.active)  return;

        glm::vec3 forward = glm::normalize(glm::vec3(cam.direction.x, cam.direction.y, 0.0f));
        glm::vec3 right   = glm::normalize(glm::cross(forward, glm::vec3(0, 0, 1)));

        trn.position += right   * vel.velocity.x * dt;
        trn.position += forward * vel.velocity.y * dt;

        if (cam.free == true)
        {
            trn.position += glm::vec3(0, 0, 1) * vel.velocity.z * dt;
        }

    });

    registry.each([&](flecs::entity ent, WINDUP_ModelCmpt& mdl, WINDUP_TransformCmpt& trn)
    {
        if (!mdl.model_handle.valid()) return;
        if (!mdl.is_visible) return;

        if (std::string_view(ent.name()) == "editor_camera")
        {
            const WINDUP_CameraCmpt* cam = ent.get<WINDUP_CameraCmpt>();
            if (cam && cam->active) return;
        }

        glm::vec3 render_rotation = trn.rotation;

        if (mdl.use_pitch)
        {
            const WINDUP_CameraCmpt* cam = ent.get<WINDUP_CameraCmpt>();
            if (cam) render_rotation.y = trn.rotation.y - cam->pitch;
        }

        WINDUP_MVP mvp{};
        mvp.model = create_model_matrix(trn.position, render_rotation, trn.scale);
        mvp.view  = view_matrix;
        mvp.proj  = proj_matrix;

        WINDUP_Model cached_model = resources->get_model(mdl.model_handle);

        for (auto& cached_mesh_handle : cached_model.desc.mesh_handles)
        {
            WINDUP_DrawCall draw_call{
                .mesh_handle = cached_mesh_handle,
                .mvp = mvp
            };
            renderer->submit_draw_call(draw_call);
        }
    });


}

void Demo::on_io_update()
{
    WINDUP_Windowing* windowing = windup->modules.windowing;
    WINDUP_Rendering* rendering = windup->modules.renderer;
    WINDUP_UserIO* user_io = windup->modules.io;
    WINDUP_ECS* ecs = windup->modules.ecs;

    flecs::world& registry = ecs->get_registry();

    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        ImGui_ImplSDL3_ProcessEvent(&e);
        ImGuiIO& imgui_io = ImGui::GetIO();


        bool cam_control_active =
            (control_mode == WINDUP_ControlMode::Scene)
            || windup->engine_configs->toggle_control_editor_camera;


        if (e.type == SDL_EVENT_KEY_DOWN &&
            (e.key.scancode == SDL_SCANCODE_F1 || e.key.scancode == SDL_SCANCODE_F2))
        {
            bool want_editor = (e.key.scancode == SDL_SCANCODE_F1);


            if (windup->engine_configs->enable_editor_mode != want_editor)
            {
                set_editor_mode(want_editor);
            }

            continue;
        }


        if (e.type == SDL_EVENT_KEY_DOWN || e.type == SDL_EVENT_KEY_UP)
        {
            if (imgui_io.WantCaptureKeyboard) continue;

            bool is_down = (e.type == SDL_EVENT_KEY_DOWN);

            if (is_down)
            {
                switch (e.key.scancode)
                {
                    case SDL_SCANCODE_F3:
                        if (windup->engine_configs->enable_editor_mode)
                        {
                            bool& cam_ctrl = windup->engine_configs->toggle_control_editor_camera;
                            cam_ctrl = !cam_ctrl;

                            windowing->configs.enable_capture_mouse = cam_ctrl;
                            SDL_SetWindowRelativeMouseMode(windowing->get_window(), cam_ctrl);

                            if (!cam_ctrl)
                            {
                                registry.each([&](WINDUP_ControllableCmpt& ctrl, WINDUP_VelocityCmpt& vel) {
                                    vel.velocity = {0, 0, 0};
                                });
                            }
                        }
                        break;

                    case SDL_SCANCODE_ESCAPE:
                        on_quit();
                        break;

                    default:
                        break;
                }
            }

            if (cam_control_active)
            {
                registry.each([&](WINDUP_ControllableCmpt& ctrl, WINDUP_VelocityCmpt& vel)
                {
                    if (!ctrl.active) return;
                    if (!cam_control_active) return;
                    float speed = is_down ? ctrl.speed : 0.0f;
                    switch (e.key.scancode)
                    {
                        case SDL_SCANCODE_W:     vel.velocity.y =  speed; break;
                        case SDL_SCANCODE_S:     vel.velocity.y = -speed; break;
                        case SDL_SCANCODE_A:     vel.velocity.x = -speed; break;
                        case SDL_SCANCODE_D:     vel.velocity.x =  speed; break;
                        case SDL_SCANCODE_SPACE: vel.velocity.z = -speed; break;
                        case SDL_SCANCODE_LCTRL: vel.velocity.z =  speed; break;
                        default: break;
                    }
                });
            }
        }

        else if (e.type == SDL_EVENT_MOUSE_MOTION)
        {
            if (!cam_control_active) continue;
            if (imgui_io.WantCaptureMouse) continue;

            float dx = e.motion.xrel * user_io->configs.sensitivity;
            float dy = e.motion.yrel * user_io->configs.sensitivity;

            registry.each([&](WINDUP_CameraCmpt& cam)
            {
                if (!cam.active) return;
                cam.yaw   -= dx;
                cam.pitch += dy;
                cam.pitch  = glm::clamp(cam.pitch, -89.0f, 89.0f);
                cam.direction = glm::normalize(glm::vec3(
                    cos(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch)),
                    sin(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch)),
                    sin(glm::radians(cam.pitch))));
            });
        }
    }
}

void Demo::on_renderer_update()
{
    WINDUP_Time* time = windup->modules.time;
    WINDUP_Profiler* profiler = windup->modules.profiler;
    WINDUP_Rendering* renderer = windup->modules.renderer;


    if (!renderer->begin_frame()) return;

    float dt = time->get_delta_time();
    profiler->frame_sample(dt);

    renderer->prepare_gui();

    if (windup->engine_configs->enable_editor_mode)
    {
        renderer->begin_base_render_pass();
        renderer->execute_render_queue();
        renderer->end_render_pass();

        renderer->begin_gui_render_pass();
        renderer->draw_gui();
        renderer->end_render_pass();
    }
    else
    {
        renderer->begin_base_render_pass();
        renderer->execute_render_queue();
        renderer->draw_gui();
        renderer->end_render_pass();
    }

    renderer->end_frame();
}

void Demo::on_level_manager_update()
{
    WINDUP_Levels* level_manager = windup->modules.level_manager;
}

void Demo::on_console_update()
{
    WINDUP_Commands* console = windup->console;

    console->flush_deferred_cmds();
}

void Demo::on_logger_update()
{
    WINDUP_Logger::print_entries();
}

//--------------------------------------------------------------------------------
// GUI
//--------------------------------------------------------------------------------

bool Demo::init_app_gui()
{
    WINDUP_Resources *resources = windup->modules.resources;
    WINDUP_Rendering *renderer  = windup->modules.renderer;

    WINDUP_GUIDesc game_gui_desc;
    game_gui_desc.name = "game_gui_style";
    ImGui::StyleColorsDark(&game_gui_desc.style);                       // baseline
    resources->load_style(base_path + "configs/game_gui_style.yaml", game_gui_desc.style);
    renderer->create_app_gui(game_gui_desc);

    renderer->submit_app_gui_cmds([this]() {
        ImGuiViewport *vp = ImGui::GetMainViewport();
        draw_hud(vp->Pos, vp->Size);
    });

    return true;
}

void Demo::draw_hud(ImVec2 scene_origin, ImVec2 scene_size)
{
    ImDrawList *dl = ImGui::GetForegroundDrawList();   // was GetWindowDrawList()
    ImFont *font = ImGui::GetFont();

    int health = 100;

    float x = scene_origin.x + 20.0f;
    float y = scene_origin.y + scene_size.y - 20.0f;

    dl->AddText(ImVec2(x, y - 70.0f), IM_COL32(150, 150, 150, 255), "HEALTH");
    dl->AddText(font, 50.0f, ImVec2(x, y - 58.0f),
                IM_COL32(255, 217, 51, 255),
                std::to_string(health).c_str());
}

//--------------------------------------------------------------------------------
// Configs
//--------------------------------------------------------------------------------

void Demo::register_app_cmds()
{
    WINDUP_Commands* console = windup->console;

    console->register_immediate_cmd("-active_camera_pos", [this](auto& args) {
        bool res = false;
        windup->modules.ecs->get_registry().each([&](WINDUP_CameraCmpt& cam, WINDUP_TransformCmpt& trn)
        {
            if (!cam.active) return;
            res = true;
            WINDUP_Logger::cmd_result("Camera", "pos: " +
                std::to_string(trn.position.x) + " " +
                std::to_string(trn.position.y) + " " +
                std::to_string(trn.position.z), 1);
        });
        return res;
    });

    console->register_immediate_cmd("-set_active_camera", [this](auto& args) {
        if (args.empty())
        {
            WINDUP_Logger::error("Console", "usage: set_active_camera <entity_name>", 0);
            return false;
        }

        const std::string& target = args[0];
        auto& registry = windup->modules.ecs->get_registry();

        flecs::entity e = registry.lookup(target.c_str());
        if (!e || !e.has<WINDUP_CameraCmpt>())
        {
            WINDUP_Logger::error("Console", "no camera entity named '" + target + "'", 0);
            return false;
        }

        registry.each([](WINDUP_CameraCmpt& cam) { cam.active = false; });
        e.get_mut<WINDUP_CameraCmpt>()->active = true;

        WINDUP_Logger::cmd_result("Camera", "Active camera set to: " + target, 0);
        return true;
    });

    console->register_immediate_cmd("-set_entity_is_visible", [this](auto& args) {
        if (args.size() < 2)
        {
            WINDUP_Logger::error("Console", "usage: set_entity_visible <entity_name> <true|false>", 0);
            return false;
        }

        const std::string& target = args[0];
        const std::string& value  = args[1];

        bool visible;
        if      (value == "true")  visible = true;
        else if (value == "false") visible = false;
        else
        {
            WINDUP_Logger::error("Console", "value must be true/false", 0);
            return false;
        }

        auto& registry = windup->modules.ecs->get_registry();
        flecs::entity e = registry.lookup(target.c_str());
        if (!e || !e.has<WINDUP_ModelCmpt>())
        {
            WINDUP_Logger::error("Console", "no entity with model named '" + target + "'", 0);
            return false;
        }

        e.get_mut<WINDUP_ModelCmpt>()->is_visible = visible;
        WINDUP_Logger::cmd_result("Entity", target + " visible = " + value, 1);
        return true;
    });

    console->register_immediate_cmd("-set_entity_pos", [this](auto& args) {
        if (args.size() < 4)
        {
            WINDUP_Logger::error("Console", "usage: set_entity_pos <entity_name> <x> <y> <z>", 1);
            return false;
        }

        const std::string& target = args[0];
        auto& registry = windup->modules.ecs->get_registry();

        flecs::entity e = registry.lookup(target.c_str());
        if (!e || !e.has<WINDUP_TransformCmpt>())
        {
            WINDUP_Logger::error("Console", "no entity named '" + target + "' with a transform", 0);
            return false;
        }

        float x, y, z;
        try
        {
            x = std::stof(args[1]);
            y = std::stof(args[2]);
            z = std::stof(args[3]);
        }
        catch (const std::exception&)
        {
            WINDUP_Logger::error("Console", "x/y/z must be numbers", 0);
            return false;
        }

        WINDUP_TransformCmpt* trn = e.get_mut<WINDUP_TransformCmpt>();
        trn->position = {x, y, z};

        WINDUP_Logger::cmd_result("Entity", "Set " + target + " position to " +
            args[1] + " " + args[2] + " " + args[3], 1);
        return true;
    });
}

std::string Demo::print_control_mode()
{
	if (control_mode == WINDUP_ControlMode::Editor) return "Editor";
	if (control_mode == WINDUP_ControlMode::Scene) return "Scene";
	return "Unknown";
}

void Demo::set_editor_mode(bool value)
{
    windup->engine_configs->enable_editor_mode = value;
    control_mode = windup->engine_configs->enable_editor_mode ? WINDUP_ControlMode::Editor : WINDUP_ControlMode::Scene;

    bool capture = !windup->engine_configs->enable_editor_mode;
    windup->modules.windowing->configs.enable_capture_mouse = capture;
    SDL_SetWindowRelativeMouseMode(windup->modules.windowing->get_window(), capture);


    auto& registry = windup->modules.ecs->get_registry();
    registry.each([&](WINDUP_CameraCmpt& cam) {
        cam.active = (cam.free == value);
    });
}


WINDUP_App* create_app() {
	return new Demo();
}