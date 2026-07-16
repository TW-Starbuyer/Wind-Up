#pragma once

// STL INCLUDES

// THIRD-PARTY INCLUDES
#include <SDL3/SDL.h>

// CORE INCLUDES
#include "../logger/logger.hpp"

// MODULE INCLUDES

// COMMON INCLUDES
#include "../../common/module.hpp"
#include "../../common/configs.hpp"

//----------------------------------------------------------------------------------------------

class WINDUP_UserIO : public WINDUP_Module
{
	public:
		WINDUP_IOConfigs configs;

		WINDUP_UserIO() = default;
		~WINDUP_UserIO() = default;

		void init(WINDUP_EngineConfigs& arg_engine_configs, WINDUP_Threading& arg_threading);
		void deinit();

	private:
		WINDUP_EngineConfigs* engine_configs = nullptr;
		WINDUP_Threading* threading = nullptr;
};

