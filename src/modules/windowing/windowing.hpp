#pragma once

// STL INCLUDES
#include <iostream>

// THIRD-PARTY INCLUDES
#include <SDL3/SDL.h>

// CORE INCLUDES
#include "../../logger/logger.hpp"

// MODULE INCLUDES
#include "../modules/devices/devices.hpp"
#include "../modules/threading/threading.hpp"

// COMMON INCLUDES
#include "../../common/alias.hpp"
#include "../../common/configs.hpp"
#include "../../common/status.hpp"
#include "../../common/math.hpp"
#include "../../common/window.hpp"
#include "../../common/cache.hpp"
#include "../../common/module.hpp"

//----------------------------------------------------------------------------------------------

class WINDUP_Windowing : public WINDUP_Module
{
	public:
		WINDUP_WindowingConfigs configs;

		WINDUP_Windowing() = default;
		~WINDUP_Windowing() = default;

		void init(WINDUP_EngineConfigs& arg_engine_configs, WINDUP_Devices& arg_devices, WINDUP_Threading& arg_threading);
		void deinit();

		bool instantiate_window(WINDUP_WindowDesc &window_desc);
		bool configure_window();
		std::tuple<int, int> get_window_dimensions();
		WINDUP_Window* get_window() { return window; }
		bool set_window(WINDUP_Window* arg_window) { window = arg_window; return true; }

	private:
		WINDUP_EngineConfigs* engine_configs = nullptr;
		WINDUP_Devices* devices = nullptr;
		WINDUP_Threading* threading = nullptr;
		WINDUP_Window* window = nullptr;
};