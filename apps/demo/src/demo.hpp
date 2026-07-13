#pragma once

#include "../../../src/windup.hpp"

enum class WINDUP_ControlMode { Editor, Scene };
enum class ActorType { EditorCamera, Box };

class Demo : public WINDUP_App
{
	public:
		WINDUP_ControlMode control_mode = WINDUP_ControlMode::Editor;

		Demo() = default;
		~Demo() = default;

		void on_run(WINDUP_EngineContext& elements_refs) override;
		void on_quit()                               override;

		void on_update()            override;
		void on_windowing_update()  override;
		void on_devices_update()    override;
		void on_resources_update()  override;
		void on_time_update()       override;
		void on_ecs_update();
		void on_io_update();
		void on_renderer_update();
		void on_level_manager_update();
		void on_console_update()	override;
		void on_logger_update()		override;

		void set_editor_mode(bool value);
		bool init_app_gui() override;
		void draw_hud(ImVec2 scene_origin, ImVec2 scene_size) override;

		void register_app_cmds()    override;

		std::string print_control_mode();
};
