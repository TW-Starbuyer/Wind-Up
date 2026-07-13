#pragma once

#include <SDL3/SDL.h>

#include "components.hpp"
#include "vertex.hpp"

#include "handles.hpp"

struct WINDUP_MeshDesc
{
	std::string name;
	std::vector<WINDUP_Vertex> vertices;
	std::vector<uint32_t> indices;
	WINDUP_MaterialHandle material_handle;
};

struct WINDUP_Mesh
{
	WINDUP_MeshDesc desc;
	WINDUP_MeshHandle handle;

	SDL_GPUBuffer *vert_buffer = nullptr;
	SDL_GPUBuffer *idx_buffer = nullptr;
};
