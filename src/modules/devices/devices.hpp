#pragma once

#include <iostream>
#include <SDL3/SDL.h>

#include "../modules/threading/threading.hpp"
#include "../../logger/logger.hpp"

#include "../../common/alias.hpp"
#include "../../common/device.hpp"
#include "../../common/configs.hpp"
#include "../../common/status.hpp"
#include "../../common/math.hpp"
#include "../../common/module.hpp"

class WINDUP_Devices : public WINDUP_Module
{
	public:
		WINDUP_DevicesConfigs configs;

		WINDUP_Devices() = default;
		~WINDUP_Devices() = default;

		void init(WINDUP_EngineConfigs& arg_engine_configs, WINDUP_Threading& arg_threading);
		void deinit();

		bool instantiate_gpu_device(WINDUP_GPUDeviceDesc &gpu_device_desc);
		WINDUP_GPUDevice* get_gpu_device() { return gpu_device; }

	private:
		WINDUP_EngineConfigs* engine_configs = nullptr;
		WINDUP_Threading* threading = nullptr;
		WINDUP_GPUDevice* gpu_device = nullptr;
};

