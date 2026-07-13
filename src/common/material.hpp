#pragma once

#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "handles.hpp"
#include "texture.hpp"

struct WINDUP_MaterialDesc
{
	std::string name;
	std::string file_path;
	WINDUP_TextureHandle base_color_texture;
	glm::vec4 base_color{1, 1, 1, 1};
};

struct WINDUP_Material
{
	WINDUP_MaterialDesc desc;
	WINDUP_MaterialHandle handle;
};