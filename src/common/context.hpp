#pragma once

#include "status.hpp"
#include "configs.hpp"

class WINDUP_Threading;
class WINDUP_Commands;
class WINDUP_Profiler;

class WINDUP_Devices;
class WINDUP_Windowing;
class WINDUP_Resources;
class WINDUP_Time;

class WINDUP_ECS;
class WINDUP_UserIO;
class WINDUP_Rendering;
class WINDUP_Levels;
class WINDUP_GraphicalInterfaces;

struct WINDUP_ModulesContext
{
	WINDUP_Time* time = nullptr;
	WINDUP_Threading* threading = nullptr;
	WINDUP_Profiler* profiler = nullptr;
	WINDUP_Devices *devices = nullptr;
	WINDUP_Resources *resources = nullptr;
	WINDUP_Windowing* windowing = nullptr;
	WINDUP_ECS *ecs = nullptr;
	WINDUP_UserIO *io = nullptr;
	WINDUP_Rendering *renderer = nullptr;
	WINDUP_Levels *level_manager = nullptr;
};

struct WINDUP_EngineContext
{
	WINDUP_EngineConfigs* engine_configs = nullptr;
	WINDUP_Commands* console = nullptr;
	WINDUP_EngineStatus *engine_status = nullptr;
	WINDUP_ModulesContext modules;
};
