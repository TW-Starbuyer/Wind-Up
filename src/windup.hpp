#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <tuple>
#include <thread>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "common/app_entry.hpp"
#include "common/alias.hpp"
#include "common/alias.hpp"
#include "common/module.hpp"
#include "common/components.hpp"
#include "common/context.hpp"
#include "common/app.hpp"
#include "common/status.hpp"
#include "common/modules.hpp"

#include "modules/threading/threading.hpp"
#include "modules/profiler/profiler.hpp"
#include "logger/logger.hpp"
#include "modules/windowing/windowing.hpp"
#include "modules/resources/resources.hpp"
#include "modules/time/time.hpp"
#include "modules/devices/devices.hpp"
#include "modules/ecs/ecs.hpp"
#include "modules/user_io/user_io.hpp"
#include "modules/renderer/renderer.hpp"
#include "modules/levels/levels.hpp"

#include "commands/commands.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

class WINDUP_ENGINE
{
	public:
		WINDUP_EngineConfigs engine_configs;
		WINDUP_EngineStatus status;

		WINDUP_EngineContext windup_ctx;

		WINDUP_Modules modules;

		WINDUP_Commands commands;

		WINDUP_ENGINE() = default;
		~WINDUP_ENGINE() = default;

		void init();
		void deinit();

		bool run();
		bool quit();

		bool load_app(WINDUP_App* app);

	private:
		std::string base_path;

		WINDUP_App *loaded_app = nullptr;

		bool init_sdl_video();
		bool init_services();
		bool init_modules();

		WINDUP_ModulesContext create_modules_ctx();

		bool register_engine_cmds();
		bool init_editor_gui();
};
