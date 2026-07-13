#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <mutex>

#include "../../common/module.hpp"
#include "../../common/status.hpp"
#include "../../common/context.hpp"

#include "../../common/thread.hpp"
#include "../../logger/logger.hpp"

#include "../../common/configs.hpp"

class WINDUP_Threading : public WINDUP_Module
{
	public:
		WINDUP_ThreadingConfigs configs;

		WINDUP_Threading()  = default;
		~WINDUP_Threading() = default;

		void init(WINDUP_EngineConfigs& arg_engine_configs);
		void deinit();

		bool spawn_thread(std::string arg_thread_name, std::function<void()> fn);

	private:
		WINDUP_EngineConfigs* engine_configs = nullptr;

		std::mutex mtx;
		std::vector<WINDUP_Thread> threads;
		std::atomic<bool> should_stop_all{false};
};