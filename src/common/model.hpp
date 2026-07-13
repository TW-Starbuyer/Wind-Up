#pragma once

#include <vector>
#include <SDL3/SDL.h>
#include "mesh.hpp"

struct WINDUP_ModelDesc
{
	std::string name;
	std::string file_path;
	std::vector<WINDUP_MeshHandle> mesh_handles;
};

struct WINDUP_Model
{
	WINDUP_ModelDesc desc;
	WINDUP_ModelHandle handle;
};
