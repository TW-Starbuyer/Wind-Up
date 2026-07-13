#include "devices.hpp"

void WINDUP_Devices::init(WINDUP_EngineConfigs& arg_engine_configs, WINDUP_Threading& arg_threading)
{
	engine_configs = &arg_engine_configs;
	threading = &arg_threading;

	WINDUP_GPUDeviceDesc gpu_device_desc{};
	instantiate_gpu_device(gpu_device_desc);

	status.f_is_init = true;
	WINDUP_Logger::task_result("Devices", "Initialization", status.f_is_init);
}

void WINDUP_Devices::deinit()
{
	status.f_is_deinit = true;

	WINDUP_Logger::task_result("Devices", "Deinitialization", status.f_is_deinit);
}

bool WINDUP_Devices::instantiate_gpu_device(WINDUP_GPUDeviceDesc &gpu_device_desc)
{
	gpu_device = SDL_CreateGPUDevice(gpu_device_desc.format_flags, gpu_device_desc.debug_mode, NULL);

	if (!gpu_device)
	{
		WINDUP_Logger::error("Devices", SDL_GetError(), 0);
		return false;
	}

	return true;
}

