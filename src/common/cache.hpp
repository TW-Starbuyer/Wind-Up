#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "status.hpp"

template<typename T, typename Handle = uint32_t>
class WINDUP_Cache
{
public:
	Handle cache_object(const std::string &name, T object)
	{
		Handle handle = {next_id++};
		object.handle = handle;
		objects[handle] = std::move(object);
		handles[name] = handle;
		return handle;
	}

	T &get_by_handle(Handle handle) { return objects.at(handle); }
	T &get_by_name(const std::string &name) { return objects.at(handles.at(name)); }

	Handle get_handle(const std::string &name) const { return handles.at(name); }
	Handle get_handle(const T &object) const { return handles.at(object.desc.name); }

	bool has(const std::string &name) const
	{
		return handles.contains(name);
	}

	bool has(Handle handle) const
	{
		auto it = objects.find(handle);
		return it != objects.end();
	}

	bool remove(const std::string &name)
	{
		auto it = handles.find(name);
		if (it == handles.end()) return false;
		objects.erase(it->second);
		handles.erase(it);
		return true;
	}

	bool remove(Handle handle)
	{
		auto it = objects.find(handle);
		if (it == objects.end()) return false;
		handles.erase(it->second.desc.name);
		objects.erase(it);
		return true;
	}

private:
	WINDUP_CacheStatus status;
	uint32_t next_id = 0;
	std::unordered_map<Handle, T> objects;
	std::unordered_map<std::string, Handle> handles;
};
