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
