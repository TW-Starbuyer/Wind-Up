#pragma once

#include <string>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

struct WINDUP_GUIDesc
{
	std::string name;
	ImGuiStyle style;
};
