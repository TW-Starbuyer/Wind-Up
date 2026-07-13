#pragma once

struct WINDUP_EngineStatus
{
	bool f_is_init = false;
	bool f_is_deinit = false;

	bool f_is_running = false;
	bool f_is_closing = false;
};

struct WINDUP_ServiceStatus
{
	bool f_is_init = false;
	bool f_is_deinit = false;
};

struct WINDUP_CoreStatus
{
	bool f_is_init = false;
	 bool f_is_deinit = false;
};

struct WINDUP_ModuleStatus
{
	bool f_is_init = false;
	bool f_is_deinit = false;
};

struct WINDUP_CacheStatus
{
	bool f_is_init = false;
	bool f_is_deinit = false;
};

struct WINDUP_AppStatus
{
	bool f_is_running = false;
	bool f_is_closing = false;
};


