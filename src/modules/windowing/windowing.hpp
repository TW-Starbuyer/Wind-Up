#pragma once

#include <iostream>
#include <SDL3/SDL.h>

#include "../modules/threading/threading.hpp"
#include "../../logger/logger.hpp"

#include "../../common/alias.hpp"
#include "../../common/configs.hpp"
#include "../../common/status.hpp"
#include "../../common/math.hpp"
#include "../../common/window.hpp"
#include "../../common/cache.hpp"

#include "../../common/module.hpp"

class WINDUP_Windowing : public WINDUP_Module
{
	public:
		WINDUP_WindowingConfigs configs;

		WINDUP_Windowing() = default;
		~WINDUP_Windowing() = default;

		void init(WINDUP_EngineConfigs& arg_engine_configs, WINDUP_Threading& arg_threading);
		void deinit();

		bool instantiate_window(WINDUP_WindowDesc &window_desc);
		bool configure_window();
		std::tuple<int, int> get_window_dimensions();
		WINDUP_Window* get_window() { return window; }
		bool set_window(WINDUP_Window* arg_window) { window = arg_window; return true; }

	private:
		WINDUP_EngineConfigs* engine_configs = nullptr;
		WINDUP_Window *window = nullptr;
};