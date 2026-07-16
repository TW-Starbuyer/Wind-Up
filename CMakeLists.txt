cmake_minimum_required(VERSION 3.20...3.31)
project("Wind-Up")

# ── Active App Configuration ─────────────────────────────────────────────────
set(ACTIVE_APP "demo" CACHE STRING "Active game/app to build")

add_executable(Wind-Up
        # Entry
        "entry.cpp"

        # Engine
        "src/windup.cpp"

        # Commands
        "src/commands/commands.cpp"

        # Core
        "src/logger/logger.cpp"

        # Modules
        "src/modules/time/time.cpp"
        "src/modules/threading/threading.cpp"
        "src/modules/profiler/profiler.cpp"
        "src/modules/devices/devices.cpp"
        "src/modules/resources/resources.cpp"
        "src/modules/windowing/windowing.cpp"
        "src/modules/ecs/ecs.cpp"
        "src/modules/user_io/user_io.cpp"
        "src/modules/renderer/renderer.cpp"
        "src/modules/levels/levels.cpp"

        # App
        "apps/${ACTIVE_APP}/src/${ACTIVE_APP}.cpp"
)

set_property(TARGET Wind-Up PROPERTY CXX_STANDARD 20)

include(FetchContent)

# ── SDL3 ────────────────────────────────────────────────────────────────────
FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-3.4.8
)
set(SDL_TESTS    OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SDL3)

# ── tinygltf ─────────────────────────────────────────────────────────────────
FetchContent_Declare(
        tinygltf
        GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
        GIT_TAG        v2.8.21
)
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE BOOL "" FORCE)
set(TINYGLTF_INSTALL               OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(tinygltf)

# ── Flecs ───────────────────────────────────────────────────────────────────
FetchContent_Declare(
        flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
        GIT_TAG        v4.0.3
)

set(FLECS_SHARED OFF CACHE BOOL "" FORCE)
set(FLECS_STATIC ON  CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(flecs)

# ── GLM ─────────────────────────────────────────────────────────────────────
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.1
)

FetchContent_MakeAvailable(glm)

# ── yaml-cpp ─────────────────────────────────────────────────────────────────
FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG        master          # no tagged release newer than 0.8.0
        GIT_SHALLOW    TRUE
)
set(YAML_CPP_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS   OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(yaml-cpp)

# ── Boost.PFR ────────────────────────────────────────────────────────────────
FetchContent_Declare(
        pfr
        GIT_REPOSITORY https://github.com/boostorg/pfr.git
        GIT_TAG        2.2.0
)
FetchContent_MakeAvailable(pfr)

# ── Dear ImGui ───────────────────────────────────────────────────────────────
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.92.6
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
        "${imgui_SOURCE_DIR}/imgui_demo.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_sdlgpu3.cpp"
)

target_include_directories(imgui PUBLIC
        "${imgui_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}/backends"
)

target_link_libraries(imgui PUBLIC SDL3::SDL3)

# ── Includes ─────────────────────────────────────────────────────────────────
target_include_directories(Wind-Up PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common"
        "${tinygltf_SOURCE_DIR}"
)

# ── Link Libraries ───────────────────────────────────────────────────────────
target_link_libraries(Wind-Up PRIVATE
        SDL3::SDL3
        flecs_static
        glm::glm-header-only
        yaml-cpp::yaml-cpp
        Boost::pfr
        imgui
)

# ── Shaders ──────────────────────────────────────────────────────────────────
find_program(GLSLC glslc)

file(GLOB HLSL_VERT_SHADERS "${CMAKE_SOURCE_DIR}/apps/${ACTIVE_APP}/assets/shaders/*.vert.hlsl")
file(GLOB HLSL_FRAG_SHADERS "${CMAKE_SOURCE_DIR}/apps/${ACTIVE_APP}/assets/shaders/*.frag.hlsl")

foreach(SHADER ${HLSL_VERT_SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    set(SPV_OUTPUT "${CMAKE_SOURCE_DIR}/apps/${ACTIVE_APP}/assets/shaders/${SHADER_NAME}.vert.spv")
    add_custom_command(
            OUTPUT ${SPV_OUTPUT}
            COMMAND ${GLSLC} -fshader-stage=vert -x hlsl ${SHADER} -o ${SPV_OUTPUT}
            DEPENDS ${SHADER}
    )
    list(APPEND SPV_FILES ${SPV_OUTPUT})
endforeach()

foreach(SHADER ${HLSL_FRAG_SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    set(SPV_OUTPUT "${CMAKE_SOURCE_DIR}/apps/${ACTIVE_APP}/assets/shaders/${SHADER_NAME}.frag.spv")
    add_custom_command(
            OUTPUT ${SPV_OUTPUT}
            COMMAND ${GLSLC} -fshader-stage=frag -x hlsl ${SHADER} -o ${SPV_OUTPUT}
            DEPENDS ${SHADER}
    )
    list(APPEND SPV_FILES ${SPV_OUTPUT})
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${SPV_FILES})
add_dependencies(Wind-Up compile_shaders)

add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/apps/${ACTIVE_APP}/assets"
        "$<TARGET_FILE_DIR:Wind-Up>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/assets"
        "$<TARGET_FILE_DIR:Wind-Up>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/apps/${ACTIVE_APP}/levels"
        "$<TARGET_FILE_DIR:Wind-Up>/levels"
        COMMENT "Copying assets + levels to build output"
)
add_dependencies(Wind-Up copy_assets)

# ── Copy configs (runs whenever build runs) ──────────────────────────────────
add_custom_target(copy_configs ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/configs"
        "$<TARGET_FILE_DIR:Wind-Up>/configs"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/apps/${ACTIVE_APP}/configs"
        "$<TARGET_FILE_DIR:Wind-Up>/configs"
        COMMENT "Copying engine + app configs to build output"
)
add_dependencies(Wind-Up copy_configs)

add_custom_command(TARGET Wind-Up POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:Wind-Up>
)