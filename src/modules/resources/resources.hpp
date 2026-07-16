#pragma once

// STL INCLUDES


// THIRD-PARTY INCLUDEs
#include <yaml-cpp/yaml.h>

// CORE INCLUDES
#include "../../logger/logger.hpp"

// MODULE INCLUDES
#include "../../modules/devices/devices.hpp"
#include "../../modules/threading/threading.hpp"

// COMMON INCLUDES
#include "../../common/model.hpp"
#include "../../common/mesh.hpp"
#include "../../common/texture.hpp"
#include "../../common/material.hpp"
#include "../../common/shader.hpp"
#include "../../common/cache.hpp"
#include "../../common/handles.hpp"
#include "../../common/module.hpp"

//----------------------------------------------------------------------------------------------

class WINDUP_Resources : public WINDUP_Module
{
	public:
		WINDUP_ResourcesConfigs configs;

		WINDUP_Resources() = default;
		~WINDUP_Resources() = default;

		void init(WINDUP_EngineConfigs& arg_configs, WINDUP_Threading& arg_threading, WINDUP_Devices& arg_devices);
		void deinit();

		// YAML
		std::optional<YAML::Node> load_yaml(const std::string &path);

		// Styles
		bool load_style(const std::string &path, ImGuiStyle &style);
		bool save_style(const std::string &path, const ImGuiStyle &style);

		// Shaders
		bool has_shader(const std::string &shader_name) { return shaders_cache.has(shader_name); }
		bool has_shader(WINDUP_ShaderHandle shader_handle) { return shaders_cache.has(shader_handle); }
		WINDUP_Shader &get_shader(WINDUP_ShaderHandle shader_handle) { return shaders_cache.get_by_handle(shader_handle); };
		WINDUP_Shader &get_shader(const std::string &shader_name) { return shaders_cache.get_by_name(shader_name); };
		WINDUP_ShaderHandle instantiate_shader(const WINDUP_ShaderDesc &shader_desc);
		void destroy_shader(WINDUP_ShaderHandle handle);

		// Textures
		bool has_texture(const std::string &texture_name) { return textures_cache.has(texture_name); }
		bool has_texture(WINDUP_TextureHandle texture_handle) { return textures_cache.has(texture_handle); }
		WINDUP_Texture &get_texture(WINDUP_TextureHandle texture_handle) { return textures_cache.get_by_handle(texture_handle); };
		WINDUP_Texture &get_texture(const std::string &texture_name) { return textures_cache.get_by_name(texture_name); };
		WINDUP_TextureHandle instantiate_texture(WINDUP_TextureDesc &texture_desc);
		void destroy_texture(WINDUP_TextureHandle handle);

		// Materials
		bool has_material(const std::string &material_name) { return materials_cache.has(material_name); }
		bool has_material(WINDUP_MaterialHandle material_handle) { return materials_cache.has(material_handle); }
		WINDUP_Material &get_material(WINDUP_MaterialHandle material_handle) { return materials_cache.get_by_handle(material_handle); };
		WINDUP_Material &get_material(const std::string &material_name) { return materials_cache.get_by_name(material_name); };
		WINDUP_MaterialHandle instantiate_material(const WINDUP_MaterialDesc &material_desc);
		void destroy_material(WINDUP_MaterialHandle handle);

		// Meshes
		bool has_mesh(const std::string &mesh_name) { return meshes_cache.has(mesh_name); }
		bool has_mesh(WINDUP_MeshHandle mesh_handle) { return meshes_cache.has(mesh_handle); }
		WINDUP_Mesh &get_mesh(WINDUP_MeshHandle mesh_handle) { return meshes_cache.get_by_handle(mesh_handle); };
		WINDUP_Mesh &get_mesh(std::string &mesh_name) { return meshes_cache.get_by_name(mesh_name); };
		WINDUP_MeshHandle instantiate_mesh(const WINDUP_MeshDesc &mesh_desc);
		void destroy_mesh(WINDUP_MeshHandle handle);

		// Models
		bool has_model(const std::string &model_name) { return models_cache.has(model_name); }
		bool has_model(WINDUP_ModelHandle model_handle) { return models_cache.has(model_handle); };
		WINDUP_Model &get_model(WINDUP_ModelHandle model_handle) { return models_cache.get_by_handle(model_handle); };
		WINDUP_Model &get_model(std::string &model_name) { return models_cache.get_by_name(model_name); };
		WINDUP_ModelHandle instantiate_model(WINDUP_ModelDesc &model_desc);
		void destroy_model(WINDUP_ModelHandle handle);

	private:
		WINDUP_EngineConfigs* engine_configs = nullptr;

		WINDUP_Devices *devices = nullptr;
		WINDUP_Cache<WINDUP_Mesh, WINDUP_MeshHandle> meshes_cache;
		WINDUP_Cache<WINDUP_Model, WINDUP_ModelHandle> models_cache;
		WINDUP_Cache<WINDUP_Texture, WINDUP_TextureHandle> textures_cache;
		WINDUP_Cache<WINDUP_Material, WINDUP_MaterialHandle> materials_cache;
		WINDUP_Cache<WINDUP_Shader, WINDUP_ShaderHandle> shaders_cache;
};
