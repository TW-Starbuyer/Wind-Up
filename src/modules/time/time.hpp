#pragma once

#include <iostream>
#include <SDL3/SDL.h>

#include "../modules/threading/threading.hpp"

#include "../../common/configs.hpp"
#include "../../common/status.hpp"

#include "../../modules/resources/resources.hpp"

#include "../../common/math.hpp"

#include "../../common/module.hpp"


class WINDUP_Time : public WINDUP_Module
{
	public:
		WINDUP_TimeConfigs configs;

		WINDUP_Time() = default;
		~WINDUP_Time() = default;

		void init(WINDUP_EngineConfigs& arg_configs);
		void deinit();

		void tick();
		double get_total_time() const { return total_time; }
		float get_delta_time() const { return delta_time; }

	private:
		WINDUP_EngineConfigs* engine_configs = nullptr;

		double total_time = 0.0;
		float delta_time = 0.0f;
		uint64_t last_tick = 0;
		uint64_t start_tick = 0;

};

