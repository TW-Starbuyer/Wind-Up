#pragma once
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL3/SDL.h>

#include <optional>
#include <type_traits>

#include <SDL3/SDL.h>
#include <boost/pfr.hpp>
#include <yaml-cpp/yaml.h>

#pragma once

#include <string>
#include <optional>
#include <type_traits>

#include <SDL3/SDL.h>
#include <boost/pfr.hpp>
#include <yaml-cpp/yaml.h>

struct WINDUP_EngineConfigs
{
    bool test  = false;
    bool debug = false;
    bool enable_editor_mode = true;
    bool enable_dev_console = true;
    bool toggle_control_editor_camera = false;
};

struct WINDUP_ProfilerConfigs
{
    bool test  = false;
    bool debug = false;
};

struct WINDUP_ThreadingConfigs
{
    bool test  = false;
    bool debug = false;
};

struct WINDUP_LoggerConfigs
{
    bool test  = false;
    bool debug = false;

    size_t max_log_entries = 1000;
    bool print_level_cmd_result  = true;
    bool print_level_info        = true;
    bool print_level_task_result = true;
    bool print_level_warning     = true;
    bool print_level_error       = true;
};

struct WINDUP_ConsoleConfigs
{
    bool test  = false;
    bool debug = false;
};

struct WINDUP_WindowingConfigs
{
    bool test  = false;
    bool debug = false;

    std::string init_title     = "Wind-Up Engine";

    int init_width  = 1920;
    int init_height = 1080;

    Uint64 init_flags = SDL_WINDOW_RESIZABLE;

    bool enable_capture_mouse = true;
};

struct WINDUP_DevicesConfigs
{
    bool test  = false;
    bool debug = false;
};

struct WINDUP_ResourcesConfigs
{
    bool test  = false;
    bool debug = false;
};

struct WINDUP_TimeConfigs
{
    bool test  = false;
    bool debug = false;
};

struct WINDUP_ECSConfigs
{
    bool test  = false;
    bool debug = false;
};

struct WINDUP_IOConfigs
{
    bool test  = false;
    bool debug = false;

    float sensitivity = 0.25f;
};

struct WINDUP_RendererConfigs
{
    bool test  = false;
    bool debug = false;

    std::string editor_background = "TARMAC_1A.PNG";
};

struct WINDUP_LevelManagerConfigs
{
    bool test  = false;
    bool debug = false;
};

struct WINDUP_GraphicalInterfacesConfigs
{
    bool test  = false;
    bool debug = false;
};

