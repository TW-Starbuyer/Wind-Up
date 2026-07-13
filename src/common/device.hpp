#pragma once

#include "handles.hpp"

#include <SDL3/SDL.h>

struct WINDUP_GPUDeviceDesc
{
	SDL_GPUShaderFormat format_flags = SDL_GPU_SHADERFORMAT_SPIRV;
	bool debug_mode = false;
};
