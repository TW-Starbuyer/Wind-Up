#pragma once

#include "../../common/module.hpp"
#include "../../common/configs.hpp"

#include <SDL3/SDL.h>

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
};

