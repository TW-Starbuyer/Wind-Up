#pragma once

#include "components.hpp"
#include "render_data.hpp"

#include <optional>

struct WINDUP_PipelineDesc
{
	std::string name;
	std::string shader_name;
	SDL_GPUPrimitiveType primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	WINDUP_RasterDepthConfigs raster_depth_configs;
};

struct WINDUP_Pipeline
{
	WINDUP_PipelineDesc desc;
	WINDUP_PipelineHandle handle;
	SDL_GPUGraphicsPipeline *sdl_pipeline = nullptr;
};

