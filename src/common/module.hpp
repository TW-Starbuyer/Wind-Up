#pragma once

#include <string>
#include "status.hpp"
#include "context.hpp"

template<typename T, typename... Args>
std::unique_ptr<T> init_module(Args &&... args)
{
	auto module = std::make_unique<T>();
	module->init(std::forward<Args>(args)...);
	if (!module->is_init())
		throw std::runtime_error(std::string(typeid(T).name()) + " initialization failure");
	return module;
}

class WINDUP_Module
{
	public:
		virtual ~WINDUP_Module() = default;
		bool is_init() const { return status.f_is_init; }
		bool is_deinit() const { return status.f_is_deinit; }

	protected:
		WINDUP_ModuleStatus status;
		std::string base_path;
};
