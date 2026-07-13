#pragma once

#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "handles.hpp"

struct WINDUP_TextureDesc
{
	std::string name;
	std::string file_path;
	int width = 0;
	int height = 0;
	std::vector<uint8_t> image_data;
};

struct WINDUP_Texture
{
	WINDUP_TextureDesc desc;
	WINDUP_TextureHandle handle;
	SDL_GPUTexture *texture = nullptr;
	SDL_GPUSampler *gpu_sampler = nullptr;
};
