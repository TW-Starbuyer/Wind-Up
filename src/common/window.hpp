#pragma once

#include "handles.hpp"

#include <SDL3/SDL.h>

struct WINDUP_WindowDesc
{
	std::string name;
	int init_width = 1280;
	int init_height = 720;
	SDL_WindowFlags init_flags = 0;
};