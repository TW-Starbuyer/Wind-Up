#pragma once

#include <memory>
class WINDUP_Time;
class WINDUP_Threading;
class WINDUP_Profiler;
class WINDUP_Devices;
class WINDUP_Resources;
class WINDUP_Windowing;
class WINDUP_ECS;
class WINDUP_UserIO;
class WINDUP_Rendering;
class WINDUP_Levels;

struct WINDUP_Modules
{
	std::unique_ptr<WINDUP_Time> time = nullptr;
	std::unique_ptr<WINDUP_Threading> threading = nullptr;
	std::unique_ptr<WINDUP_Profiler> profiler = nullptr;
	std::unique_ptr<WINDUP_Devices> devices = nullptr;
	std::unique_ptr<WINDUP_Resources> resources = nullptr;
	std::unique_ptr<WINDUP_Windowing> windowing = nullptr;
	std::unique_ptr<WINDUP_ECS> ecs = nullptr;
	std::unique_ptr<WINDUP_UserIO> io = nullptr;
	std::unique_ptr<WINDUP_Rendering> renderer = nullptr;
	std::unique_ptr<WINDUP_Levels> levels = nullptr;
};