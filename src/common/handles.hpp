#pragma once

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>


struct WINDUP_EntityHandle
{
	uint32_t id = UINT32_MAX;
	bool valid() const { return id != UINT32_MAX; }

	bool operator==(const WINDUP_EntityHandle &other) const { return id == other.id; }
};

template<>
struct std::hash<WINDUP_EntityHandle>
{
	size_t operator()(const WINDUP_EntityHandle &h) const
	{
		return std::hash<uint32_t>{}(h.id);
	}
};

struct WINDUP_MeshHandle
{
	uint32_t id = UINT32_MAX;
	bool valid() const { return id != UINT32_MAX; }

	bool operator==(const WINDUP_MeshHandle &other) const { return id == other.id; }
};

template<>
struct std::hash<WINDUP_MeshHandle>
{
	size_t operator()(const WINDUP_MeshHandle &h) const
	{
		return std::hash<uint32_t>{}(h.id);
	}
};

struct WINDUP_ModelHandle
{
	uint32_t id = UINT32_MAX;
	bool valid() const { return id != UINT32_MAX; }

	bool operator==(const WINDUP_ModelHandle &other) const { return id == other.id; }
};

template<>
struct std::hash<WINDUP_ModelHandle>
{
	size_t operator()(const WINDUP_ModelHandle &h) const
	{
		return std::hash<uint32_t>{}(h.id);
	}
};

struct WINDUP_ShaderHandle
{
	uint32_t id = UINT32_MAX;
	bool valid() const { return id != UINT32_MAX; }

	bool operator==(const WINDUP_ShaderHandle &other) const { return id == other.id; }
};

template<>
struct std::hash<WINDUP_ShaderHandle>
{
	size_t operator()(const WINDUP_ShaderHandle &h) const
	{
		return std::hash<uint32_t>{}(h.id);
	}
};

struct WINDUP_TextureHandle
{
	uint32_t id = UINT32_MAX;
	bool valid() const { return id != UINT32_MAX; }

	bool operator==(const WINDUP_TextureHandle &other) const { return id == other.id; }
};

template<>
struct std::hash<WINDUP_TextureHandle>
{
	size_t operator()(const WINDUP_TextureHandle &h) const
	{
		return std::hash<uint32_t>{}(h.id);
	}
};

struct WINDUP_MaterialHandle
{
	uint32_t id = UINT32_MAX;
	bool valid() const { return id != UINT32_MAX; }

	bool operator==(const WINDUP_MaterialHandle &other) const { return id == other.id; }
};

template<>
struct std::hash<WINDUP_MaterialHandle>
{
	size_t operator()(const WINDUP_MaterialHandle &h) const
	{
		return std::hash<uint32_t>{}(h.id);
	}
};

struct WINDUP_PipelineHandle
{
	uint32_t id = UINT32_MAX;
	bool valid() const { return id != UINT32_MAX; }

	bool operator==(const WINDUP_PipelineHandle &other) const { return id == other.id; }
};

template<>
struct std::hash<WINDUP_PipelineHandle>
{
	size_t operator()(const WINDUP_PipelineHandle &h) const
	{
		return std::hash<uint32_t>{}(h.id);
	}
};

struct WINDUP_LevelHandle
{
	uint32_t id = UINT32_MAX;
	bool valid() const { return id != UINT32_MAX; }

	bool operator==(const WINDUP_LevelHandle &other) const { return id == other.id; }
};

template<>
struct std::hash<WINDUP_LevelHandle>
{
	size_t operator()(const WINDUP_LevelHandle &h) const
	{
		return std::hash<uint32_t>{}(h.id);
	}
};

struct WINDUP_WindowHandle
{
	uint32_t id = UINT32_MAX;
	bool valid() const { return id != UINT32_MAX; }

	bool operator==(const WINDUP_WindowHandle &other) const { return id == other.id; }
};

template<>
struct std::hash<WINDUP_WindowHandle>
{
	size_t operator()(const WINDUP_WindowHandle &h) const
	{
		return std::hash<uint32_t>{}(h.id);
	}
};
