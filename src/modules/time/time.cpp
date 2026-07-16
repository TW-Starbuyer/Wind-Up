#include "time.hpp"

void WINDUP_Time::init(WINDUP_EngineConfigs& arg_configs)
{
	engine_configs = &arg_configs;
	last_tick = SDL_GetTicksNS();

	status.f_is_init = true;
	WINDUP_Logger::task_result("Time", "Initialization", status.f_is_init);
}

void WINDUP_Time::deinit()
{
	status.f_is_deinit = true;
	WINDUP_Logger::task_result("Time", "Deinitialization", status.f_is_deinit);
}

// Currently using old school variable time step but need to read more about trade-offs vs fixed.
void WINDUP_Time::tick()
{
	uint64_t now = SDL_GetTicksNS();
	delta_time = (float) (now - last_tick) / 1e9f; // Simple divison by a billion to convert ns to seconds. Thanks S.I.
	total_time += delta_time;
	last_tick = now;
}
