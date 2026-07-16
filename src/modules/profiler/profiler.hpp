#pragma once

// STL INCLUDES
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <mutex>

// CORE INCLUDES
#include "../logger/logger.hpp"

// MODULE INCLUDES
#include "../../modules/profiler/profiler.hpp"

// COMMON INCLUDES
#include "../../common/status.hpp"
#include "../../common/context.hpp"
#include "../../common/module.hpp"

//----------------------------------------------------------------------------------------------

class WINDUP_Profiler : public WINDUP_Module
{
	public:
		static constexpr int FPS_HISTORY = 120;

		WINDUP_ProfilerConfigs profiler_configs;

		WINDUP_Profiler()  = default;
		~WINDUP_Profiler() = default;

		void init(WINDUP_EngineConfigs& arg_engine_configs);
		void deinit();

		template<typename Fn>
		void time_it(const std::string& category, const std::string& name, Fn&& fn)
		{
			Uint64 start = SDL_GetPerformanceCounter();
			fn();
			Uint64 end = SDL_GetPerformanceCounter();
			double ms = (end - start) * 1000.0 / SDL_GetPerformanceFrequency();
			perf_times[category + "/" + name] += ms;
			WINDUP_Logger::info(category, name + ": " + std::to_string(ms) + "ms", 1);
		}

		void frame_sample(float dt);

		// Of course with profiling you want read only behavior, just some compiler saftey with ensuring const return
		const float* fps_data() const;
		int fps_count() const;
		float fps_latest() const;
		float fps_average() const;

	private:
		WINDUP_EngineConfigs* engine_configs = nullptr;
		std::unordered_map<std::string, double> perf_times;
		std::vector<float> fps_history;
};