#include "windowing.hpp"

void WINDUP_Windowing::init(WINDUP_EngineConfigs& arg_engine_configs, WINDUP_Threading& arg_threading)
{
	engine_configs = &arg_engine_configs;
	WINDUP_WindowDesc window_desc = {
		configs.init_title, configs.init_width, configs.init_height, configs.init_flags,
	};

	instantiate_window(window_desc);

	configure_window();

	status.f_is_init = true;

	WINDUP_Logger::task_result("Windowing", "Initialized", status.f_is_init);
}

void WINDUP_Windowing::deinit()
{
	status.f_is_deinit = true;

	WINDUP_Logger::task_result("Windowing", "Deinitialized", status.f_is_deinit);
}

bool WINDUP_Windowing::instantiate_window(WINDUP_WindowDesc &window_desc)
{
	SDL_Window *sdl_window = SDL_CreateWindow(
		window_desc.name.c_str(),
		window_desc.init_width,
		window_desc.init_height,
		window_desc.init_flags
	);
	if (!sdl_window)
	{
		WINDUP_Logger::error("Windowing", SDL_GetError(), 0);
		return false;
	}

	window = sdl_window;

	return true;
}

bool WINDUP_Windowing::configure_window()
{
	bool editor   = engine_configs->enable_editor_mode;
	bool cam_ctrl = engine_configs->toggle_control_editor_camera;
	configs.enable_capture_mouse = !editor || cam_ctrl;
	SDL_SetWindowRelativeMouseMode(window, configs.enable_capture_mouse);
	return true;
}

std::tuple<int, int> WINDUP_Windowing::get_window_dimensions()
{
	int w = 0;
	int h = 0;
	SDL_GetWindowSize(window, &w, &h);
	return {w, h};
}