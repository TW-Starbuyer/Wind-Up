#pragma once

// STL INCLUDES

// THIRD-PARTY INCLUDES
#include <flecs.h>

// CORE INCLUDES
#include "../logger/logger.hpp"

// MODULE INCLUDES
#include "../modules/threading/threading.hpp"

// COMMON INCLUDES
#include "../../common/module.hpp"
#include "../../common/configs.hpp"
#include "../../common/components.hpp"
#include "../../common/entity.hpp"

//----------------------------------------------------------------------------------------------

class WINDUP_ECS : public WINDUP_Module
{
	public:
		WINDUP_ECSConfigs configs;

		WINDUP_ECS() = default;
		~WINDUP_ECS() = default;

		void init(WINDUP_EngineConfigs& arg_engine_configs, WINDUP_Threading& arg_threading);
		void deinit();

		// Registry
		flecs::world& get_registry() { return registry; };
		bool set_registry(flecs::world arg_registry) { registry = arg_registry; return true; };

		// Entities
		bool has_entity(const std::string &entity_name) { return registry.lookup(entity_name.c_str()).is_valid(); }
		flecs::entity spawn_entity(const WINDUP_EntityDesc &entity_desc);

	private:
		WINDUP_EngineConfigs* engine_configs = nullptr;

		flecs::world registry;
};