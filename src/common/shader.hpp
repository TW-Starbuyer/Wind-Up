#pragma once

#include "handles.hpp"

#include <SDL3/SDL.h>

struct WINDUP_ShaderDesc
{
	std::string name;
	std::string vert_path;
	std::string frag_path;
	SDL_GPUFillMode fill_mode = SDL_GPU_FILLMODE_FILL;
	SDL_GPUCullMode cull_mode = SDL_GPU_CULLMODE_NONE;
	SDL_GPUFrontFace front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
	bool depth_test_enable = true;
	bool depth_write_enable = true;
	SDL_GPUCompareOp depth_compare_op = SDL_GPU_COMPAREOP_LESS;
};

struct WINDUP_Shader
{
	WINDUP_ShaderDesc desc;
	WINDUP_ShaderHandle handle;
	SDL_GPUShader *vert = nullptr;
	SDL_GPUShader *frag = nullptr;
};