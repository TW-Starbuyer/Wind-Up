#pragma once

#include <string>

#include "context.hpp"

class WINDUP_App
{
public:
	virtual ~WINDUP_App() = default;

	virtual void on_run(WINDUP_EngineContext &elements_refs) = 0;
	virtual void on_quit() = 0;

	virtual void on_windowing_update() = 0;
	virtual void on_devices_update() = 0;
	virtual void on_resources_update() = 0;
	virtual void on_time_update() = 0;
	virtual void on_console_update() = 0;
	virtual void on_logger_update() = 0;
	virtual void on_update() = 0;

	virtual void register_app_cmds() = 0;
	virtual bool init_app_gui() = 0;
	virtual void draw_hud(ImVec2 scene_origin, ImVec2 scene_size) = 0;

	WINDUP_AppStatus status;
	WINDUP_EngineContext *windup = nullptr;
	std::string base_path;
};
