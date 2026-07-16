#include "profiler.hpp"

void WINDUP_Profiler::init(WINDUP_EngineConfigs& arg_engine_configs)
{
	engine_configs = &arg_engine_configs;

	status.f_is_init = true;
}

void WINDUP_Profiler::deinit()
{
	status.f_is_deinit = true;
}

void WINDUP_Profiler::frame_sample(float dt)
{
	float fps = (dt > 0.0f) ? 1.0f / dt : 0.0f;
	fps_history.push_back(fps);
	if (fps_history.size() > FPS_HISTORY)
	{
		fps_history.erase(fps_history.begin());
	}
}

const float* WINDUP_Profiler::fps_data() const
{
	return fps_history.data();
}

int WINDUP_Profiler::fps_count() const
{
	return (int)fps_history.size();
}

float WINDUP_Profiler::fps_latest() const
{
	return fps_history.empty() ? 0.0f : fps_history.back();
}

float WINDUP_Profiler::fps_average() const
{
	if (fps_history.empty()) return 0.0f;
	float sum = 0.0f;
	for (float v : fps_history) sum += v;
	return sum / fps_history.size();
}
