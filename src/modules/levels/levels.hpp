#pragma once

#include <optional>
#include <boost/pfr.hpp>
#include <yaml-cpp/yaml.h>

#include "../../logger/logger.hpp"
#include "../../common/configs.hpp"
#include "../../common/module.hpp"
#include "../../common/cache.hpp"

#include "../../modules/resources/resources.hpp"

#include "../../modules/renderer/renderer.hpp"
#include "../../modules/ecs/ecs.hpp"

#include "../../common/context.hpp"

#include "../../common/level.hpp"
#include "../../common/shader.hpp"

class WINDUP_Levels : public WINDUP_Module
{
    public:
        WINDUP_LevelManagerConfigs configs;

        WINDUP_Levels() = default;
        ~WINDUP_Levels() = default;

        void init(WINDUP_EngineConfigs& arg_engine_configs, WINDUP_Threading& arg_threading, WINDUP_Resources& arg_resources, WINDUP_ECS &arg_ecs, WINDUP_Rendering &arg_renderer);
        void deinit();

        std::optional<WINDUP_LevelDesc> load_level_from_file(const std::string &arg_path);
        WINDUP_LevelHandle load_level(const std::string &path);
        WINDUP_LevelHandle instantiate_level(const WINDUP_LevelDesc &level_desc);
        void set_active_level(WINDUP_LevelHandle handle) { active_level = handle; }
        void set_active_level(const std::string &name) { active_level = levels_cache.get_by_name(name).handle; }
        WINDUP_Level &get_level(WINDUP_LevelHandle handle) { return levels_cache.get_by_handle(handle); }
        WINDUP_Level &get_level(const std::string &name) { return levels_cache.get_by_name(name); }
        WINDUP_Level &get_active_level() { return levels_cache.get_by_handle(active_level); }

    private:
        WINDUP_EngineConfigs* engine_configs = nullptr;
        WINDUP_Resources* resources = nullptr;
        WINDUP_ECS *ecs = nullptr;
        WINDUP_Rendering *renderer = nullptr;
        WINDUP_Cache<WINDUP_Level, WINDUP_LevelHandle> levels_cache;
        WINDUP_LevelHandle active_level{};

        glm::vec3 parse_vec3(const YAML::Node &node);
        std::optional<WINDUP_TransformCmpt> parse_transform(const YAML::Node &node);
        std::optional<WINDUP_CameraCmpt> parse_camera(const YAML::Node &node);
        std::optional<WINDUP_VelocityCmpt> parse_velocity(const YAML::Node &node);
        std::optional<WINDUP_ControllableCmpt> parse_controllable(const YAML::Node &node);
        std::optional<WINDUP_ModelCmpt> parse_model(const YAML::Node &node);
};