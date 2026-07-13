#include "ecs.hpp"
#include "../../modules/resources/resources.hpp"

void WINDUP_ECS::init(WINDUP_EngineConfigs& arg_engine_configs, WINDUP_Threading& arg_threading)
{
	engine_configs = &arg_engine_configs;

	// load_configs(*services->resources, "configs/configs_world.json");

	registry.component<WINDUP_TransformCmpt>();
	registry.component<WINDUP_VelocityCmpt>();
	registry.component<WINDUP_MeshCmpt>();
	registry.component<WINDUP_ModelCmpt>();
	registry.component<WINDUP_CameraCmpt>();
	registry.component<WINDUP_ControllableCmpt>();

	status.f_is_init = true;

	WINDUP_Logger::task_result("ECS", "Initialization", status.f_is_init);
}

void WINDUP_ECS::deinit()
{
	status.f_is_deinit = true;

	WINDUP_Logger::task_result("ECS", "Deinitialization", status.f_is_deinit);
}

flecs::entity WINDUP_ECS::spawn_entity(const WINDUP_EntityDesc &entity_desc)
{
	// Create flecs entity (named or anonymous)
	flecs::entity e = entity_desc.name.empty()
		                  ? registry.entity()
		                  : registry.entity(entity_desc.name.c_str());

	// Set components if present in descriptor
	if (entity_desc.model) e.set<WINDUP_ModelCmpt>(*entity_desc.model);
	if (entity_desc.transform) e.set<WINDUP_TransformCmpt>(*entity_desc.transform);
	if (entity_desc.camera) e.set<WINDUP_CameraCmpt>(*entity_desc.camera);
	if (entity_desc.controllable) e.set<WINDUP_ControllableCmpt>(*entity_desc.controllable);
	if (entity_desc.velocity) e.set<WINDUP_VelocityCmpt>(*entity_desc.velocity);

	return e;
}