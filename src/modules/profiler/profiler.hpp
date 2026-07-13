#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <mutex>

#include "status.hpp"
#include "context.hpp"

#include "../../common/module.hpp"
#include "../../modules/profiler/profiler.hpp"
#include "../logger/logger.hpp"

class WINDUP_Profiler : public WINDUP_Module
{
	public:
		WINDUP_ProfilerConfigs profiler_configs;

		WINDUP_Profiler()  = default;
		~WINDUP_Profiler() = default;

		void init(WINDUP_EngineConfigs& arg_engine_configs);
		void deinit();

		template<typename Fn>
		void measure_duration(const std::string& category, const std::string& name, Fn&& fn)
		{
			Uint64 start = SDL_GetPerformanceCounter();
			fn();
			Uint64 end = SDL_GetPerformanceCounter();
			double ms = (end - start) * 1000.0 / SDL_GetPerformanceFrequency();
			perf_times[category + "/" + name] += ms;
			WINDUP_Logger::info(category, name + ": " + std::to_string(ms) + "ms", 1);
		}

		void frame_sample(float dt)
		{
			float fps = (dt > 0.0f) ? 1.0f / dt : 0.0f;
			fps_history.push_back(fps);
			if (fps_history.size() > FPS_HISTORY)
				fps_history.erase(fps_history.begin());
		}

		static constexpr int FPS_HISTORY = 120;

		const float* fps_data()  const
		{
			return fps_history.data();
		}

		int fps_count() const
		{
			return (int)fps_history.size();
		}

		float fps_latest()  const
		{
			return fps_history.empty() ? 0.0f : fps_history.back();
		}

		float fps_average() const
		{
			if (fps_history.empty()) return 0.0f;
			float sum = 0.0f;
			for (float v : fps_history) sum += v;
			return sum / fps_history.size();
		}

	private:
		WINDUP_EngineConfigs* engine_configs = nullptr;
		std::unordered_map<std::string, double> perf_times;
		std::vector<float> fps_history;
};