#pragma once

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

#include "model.hpp"

#include "handles.hpp"

struct WINDUP_TransformCmpt
{
	glm::vec3 position{0.0f};
	glm::vec3 rotation{0.0f};
	glm::vec3 scale{1.0f};
};

struct WINDUP_MeshCmpt
{
	WINDUP_MeshHandle mesh_handle;
};

struct WINDUP_ModelCmpt
{
	WINDUP_ModelHandle model_handle;
	std::string model_filename;
	bool is_visible = true;
	bool use_pitch = false;
};

struct WINDUP_ControllableCmpt
{
	bool active = false;
	float speed = 1.0f;
};

struct WINDUP_VelocityCmpt
{
	glm::vec3 velocity{0.0f};
};

struct WINDUP_CameraCmpt
{
	bool free = false;
	float fov_degrees = 60;
	float near_plane = 0.1f;
	float far_plane = 100.0f;
	bool active = false;
	glm::vec3 direction = {0.0f, 1.0f, 0.0f};
	float yaw = 0.0f;
	float yaw_offset = 0.0f;
	float pitch = 0.0f;
	float eye_height = 0.0f;
	float eye_offset = 0.0f;
};
