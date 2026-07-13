#pragma once

#include <flecs.h>
#include <optional>
#include <string>
#include <unordered_map>

#include "../common/handles.hpp"
#include "../common/components.hpp"

struct WINDUP_EntityDesc
{
	std::string name;

	std::optional<WINDUP_ModelCmpt> model;
	std::optional<WINDUP_TransformCmpt> transform;
	std::optional<WINDUP_CameraCmpt> camera;
	std::optional<WINDUP_ControllableCmpt> controllable;
	std::optional<WINDUP_VelocityCmpt> velocity;
};
