#include "user_io.hpp"
#include "../../modules/resources/resources.hpp"
#include <iostream>

#include "../../modules/windowing/windowing.hpp"

//--------------------------------------------------------------------------------
// Init / Deinit
//--------------------------------------------------------------------------------

void WINDUP_UserIO::init(WINDUP_EngineConfigs& arg_engine_configs, WINDUP_Threading& arg_threading)
{
	status.f_is_init = true;

	WINDUP_Logger::task_result("User I/O", "Initialization", status.f_is_init);
}

void WINDUP_UserIO::deinit()
{
	status.f_is_deinit = true;

	WINDUP_Logger::task_result("User I/O", "Deinitialization", status.f_is_deinit);
}
