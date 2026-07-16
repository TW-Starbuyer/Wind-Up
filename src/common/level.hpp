#pragma once

#include <vector>

#include "../common/components.hpp"

#include "../common/pipeline.hpp"

#include "../common/entity.hpp"

#include "../common/handles.hpp"


struct WINDUP_LevelDesc
{
	std::string name;
	std::vector<WINDUP_ShaderDesc> shader_descs;
	std::vector<WINDUP_PipelineDesc> pipelines_descs;
	std::vector<WINDUP_EntityDesc> entity_descs;
};

struct WINDUP_Level
{
	WINDUP_LevelDesc desc;
	WINDUP_LevelHandle handle;

	std::vector<WINDUP_ShaderHandle> shader_handles;
	std::vector<WINDUP_PipelineHandle> pipeline_handles;
	std::vector<WINDUP_EntityHandle> entity_handles;
};
