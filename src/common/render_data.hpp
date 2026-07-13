#pragma once

#include "handles.hpp"

#include "../../common/mvp.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

#include "alias.hpp"

#include <SDL3/SDL.h>

struct WINDUP_RasterDepthConfigs
{
	bool depth_test_enable = true;
	bool depth_write_enable = true;
	SDL_GPUFillMode fill_mode = SDL_GPU_FILLMODE_FILL;
	SDL_GPUCullMode cull_mode = SDL_GPU_CULLMODE_NONE;
	SDL_GPUFrontFace front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
	SDL_GPUCompareOp depth_compare_op = SDL_GPU_COMPAREOP_LESS;
};

struct WINDUP_DrawCall
{
	WINDUP_MeshHandle mesh_handle;
	WINDUP_MVP mvp;
	WINDUP_PipelineHandle pipeline_handle;
};

struct WINDUP_SceneTarget
{
	SDL_GPUTexture *color = nullptr;
	SDL_GPUTexture *depth = nullptr;
	uint32_t w = 0;
	uint32_t h = 0;
};

struct WINDUP_RendererResources
{
	SDL_GPUSampler *default_sampler = nullptr;
	SDL_GPUTexture *fallback_texture = nullptr;
	SDL_GPUTexture *depth_texture = nullptr;
	WINDUP_SceneTarget scene_target;
};

struct WINDUP_FrameState
{
	SDL_GPUCommandBuffer *cmd_buffer = nullptr;
	SDL_GPUTexture *swapchain = nullptr;
	SDL_GPURenderPass *render_pass = nullptr;
	SDL_GPUGraphicsPipeline *pipeline = nullptr;
	ImDrawData *imgui_draw_data = nullptr;
};