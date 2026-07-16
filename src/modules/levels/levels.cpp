#include "levels.hpp"


void WINDUP_Levels::init(WINDUP_EngineConfigs& arg_engine_configs, WINDUP_Threading& arg_threading, WINDUP_Resources& arg_resources, WINDUP_ECS &arg_ecs, WINDUP_Rendering &arg_renderer)
{
    base_path = SDL_GetBasePath();

    engine_configs = &arg_engine_configs;

    resources = &arg_resources;
    ecs = &arg_ecs;
    renderer = &arg_renderer;

    status.f_is_init = true;

    WINDUP_Logger::task_result("Level Manager", "Initialization", status.f_is_init);
}

void WINDUP_Levels::deinit()
{
    status.f_is_deinit = true;

    WINDUP_Logger::task_result("Level Manager", "Deinitialization", status.f_is_deinit);
}

glm::vec3 WINDUP_Levels::parse_vec3(const YAML::Node &node)
{
    glm::vec3 result{0.0f, 0.0f, 0.0f};
    if (node && node.IsSequence() && node.size() >= 3)
    {
        result.x = node[0].as<float>(0.0f);
        result.y = node[1].as<float>(0.0f);
        result.z = node[2].as<float>(0.0f);
    }
    return result;
}

std::optional<WINDUP_TransformCmpt> WINDUP_Levels::parse_transform(const YAML::Node &node)
{
    if (!node || !node.IsMap()) return std::nullopt;

    WINDUP_TransformCmpt t{};
    if (node["position"]) t.position = parse_vec3(node["position"]);
    if (node["rotation"]) t.rotation = parse_vec3(node["rotation"]);
    if (node["scale"])    t.scale    = parse_vec3(node["scale"]);

    return t;
}

std::optional<WINDUP_CameraCmpt> WINDUP_Levels::parse_camera(const YAML::Node &node)
{
    if (!node || !node.IsMap()) return std::nullopt;

    WINDUP_CameraCmpt c{};
    if (node["free"])        c.free        = node["free"].as<bool>(c.free);
    if (node["eye_height"])  c.eye_height  = node["eye_height"].as<float>(c.eye_height);
    if (node["eye_offset"])  c.eye_offset  = node["eye_offset"].as<float>(c.eye_offset);
    if (node["fov_degrees"]) c.fov_degrees = node["fov_degrees"].as<float>(c.fov_degrees);
    if (node["near_plane"])  c.near_plane  = node["near_plane"].as<float>(c.near_plane);
    if (node["far_plane"])   c.far_plane   = node["far_plane"].as<float>(c.far_plane);
    if (node["active"])      c.active      = node["active"].as<bool>(c.active);
    if (node["direction"])   c.direction   = parse_vec3(node["direction"]);
    if (node["yaw"])        c.yaw        = node["yaw"].as<float>(c.yaw);
    if (node["yaw_offset"]) c.yaw_offset = node["yaw_offset"].as<float>(c.yaw_offset);
    if (node["pitch"])      c.pitch      = node["pitch"].as<float>(c.pitch);

    return c;
}

std::optional<WINDUP_VelocityCmpt> WINDUP_Levels::parse_velocity(const YAML::Node &node)
{
    if (!node || !node.IsMap()) return std::nullopt;

    WINDUP_VelocityCmpt v{};
    if (node["velocity"]) v.velocity = parse_vec3(node["velocity"]);

    return v;
}

std::optional<WINDUP_ControllableCmpt> WINDUP_Levels::parse_controllable(const YAML::Node &node)
{
    if (!node || !node.IsMap()) return std::nullopt;

    WINDUP_ControllableCmpt c{};
    if (node["active"]) c.active = node["active"].as<bool>(c.active);
    if (node["speed"])  c.speed  = node["speed"].as<float>(c.speed);

    return c;
}

std::optional<WINDUP_ModelCmpt> WINDUP_Levels::parse_model(const YAML::Node &node)
{
    if (!node || !node.IsMap()) return std::nullopt;

    WINDUP_ModelCmpt m{};
    if (node["model_filename"]) m.model_filename = node["model_filename"].as<std::string>();
    if (node["is_visible"]) m.is_visible = node["is_visible"].as<bool>(m.is_visible);
    if (node["use_pitch"]) m.use_pitch = node["use_pitch"].as<bool>(m.use_pitch);

    return m;
}

WINDUP_LevelHandle WINDUP_Levels::instantiate_level(const WINDUP_LevelDesc &level_desc)
{
    // Return if cache hit.
    if (levels_cache.has(level_desc.name))
    {
        return levels_cache.get_by_name(level_desc.name).handle;
    }

    flecs::world &registry = ecs->get_registry();

    // Initial level object creation and caching.
    WINDUP_Level level;
    level.desc = level_desc;
    WINDUP_LevelHandle handle = levels_cache.cache_object(level_desc.name, level);
    WINDUP_Level& cached_level = levels_cache.get_by_handle(handle);
    cached_level.handle = handle;

    // Instantiate shader if not cached, then cache.
    for (const auto &shader_desc: level_desc.shader_descs)
    {
        if (!resources->has_shader(shader_desc.name))
        {
            WINDUP_ShaderHandle shader_handle = resources->instantiate_shader(shader_desc);
            cached_level.shader_handles.push_back(shader_handle);
        }
    }

    // Create pipelines
    for (const auto &pipeline_desc: level_desc.pipelines_descs)
    {
        renderer->create_pipeline(pipeline_desc);
    }

    // Spawn entities
    registry.defer_begin();
    for (const auto &entity_desc: level_desc.entity_descs)
    {
        flecs::entity e = ecs->spawn_entity(entity_desc);

        if (entity_desc.model)
        {
            std::string model_filename = entity_desc.model->model_filename;
            std::string base_path = SDL_GetBasePath();
            WINDUP_ModelHandle model_handle;

            if (resources->has_model(model_filename))
            {
                model_handle = resources->get_model(model_filename).handle;
                if (!model_handle.valid())
                {
                    SDL_Log("ERROR: cached model '%s' has invalid handle", model_filename.c_str());
                    continue;
                }
            }
            else
            {
                WINDUP_ModelDesc model_desc{
                    .name = model_filename,
                    .file_path = base_path + "assets/models/" + model_filename + ".glb"
                };
                model_handle = resources->instantiate_model(model_desc);

                if (!model_handle.valid())
                {
                    SDL_Log("ERROR: Model instantiation failed for %s", model_filename.c_str());
                    continue;
                }

                renderer->upload_model(model_handle);
            }

            WINDUP_ModelCmpt model_cmpt = *entity_desc.model;
            model_cmpt.model_handle = model_handle;
            e.set<WINDUP_ModelCmpt>(model_cmpt);
        }
    }
    registry.defer_end();

    // Set as active level
    active_level = handle;

    WINDUP_Logger::task_result("Instantiate Level", "[" + level_desc.name + "]", 1);

    return handle;
}

std::optional<WINDUP_LevelDesc> WINDUP_Levels::load_level_from_file(const std::string &arg_path)
{
    std::string path = base_path + "levels/" + arg_path + ".yaml";
    auto yaml_opt = resources->load_yaml(path);
    if (!yaml_opt)
    {
        SDL_Log("Failed to load level from: %s", path.c_str());
        return std::nullopt;
    }

    const YAML::Node &root = *yaml_opt;

    // Extract level name first to check cache
    if (!root["level_name"])
    {
        SDL_Log("Level file missing 'level_name' field");
        return std::nullopt;
    }
    std::string level_name = root["level_name"].as<std::string>();

    // Check cache - if already loaded, return cached desc
    if (levels_cache.has(level_name))
    {
        SDL_Log("Level '%s' already loaded, returning cached version", level_name.c_str());
        return levels_cache.get_by_name(level_name).desc;
    }

    WINDUP_LevelDesc level_desc;
    level_desc.name = level_name;

    // Parse shader descs
    if (const YAML::Node &shaders = root["shader_descs"]; shaders && shaders.IsSequence())
    {
        for (const auto &shader: shaders)
        {
            WINDUP_ShaderDesc desc;
            if (shader["shader_name"])       desc.name = shader["shader_name"].as<std::string>();
            if (shader["vert_shader_path"])  desc.vert_path   = shader["vert_shader_path"].as<std::string>();
            if (shader["frag_shader_path"])  desc.frag_path   = shader["frag_shader_path"].as<std::string>();
            level_desc.shader_descs.push_back(desc);
        }
    }

    // Parse pipeline descs
    if (const YAML::Node &pipelines = root["pipelines_descs"]; pipelines && pipelines.IsSequence())
    {
        for (const auto &pipeline: pipelines)
        {
            WINDUP_PipelineDesc desc;
            if (pipeline["pipeline_name"]) desc.name = pipeline["pipeline_name"].as<std::string>();
            if (pipeline["shader_name"])   desc.shader_name   = pipeline["shader_name"].as<std::string>();
            level_desc.pipelines_descs.push_back(desc);
        }
    }

    // Parse entity descs
    if (const YAML::Node &entities = root["entity_descs"]; entities && entities.IsSequence())
    {
        for (const auto &entity: entities)
        {
            WINDUP_EntityDesc desc;

            if (entity["entity_name"])    desc.name    = entity["entity_name"].as<std::string>();

            // Parse components
            if (entity["model"])        desc.model        = parse_model(entity["model"]);
            if (entity["transform"])    desc.transform    = parse_transform(entity["transform"]);
            if (entity["camera"])       desc.camera       = parse_camera(entity["camera"]);
            if (entity["velocity"])     desc.velocity     = parse_velocity(entity["velocity"]);
            if (entity["controllable"]) desc.controllable = parse_controllable(entity["controllable"]);

            level_desc.entity_descs.push_back(desc);
        }
    }

    return level_desc;
}

WINDUP_LevelHandle WINDUP_Levels::load_level(const std::string &path)
{
    auto level_desc_opt = load_level_from_file(path);

    if (!level_desc_opt)
    {
        SDL_Log("ERROR: Failed to load level: %s", path.c_str());
        return {}; // return invalid handle
    }

    return instantiate_level(*level_desc_opt);
}