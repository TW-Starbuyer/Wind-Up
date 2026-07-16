#include "threading.hpp"

void WINDUP_Threading::init(WINDUP_EngineConfigs& arg_engine_configs)
{
	engine_configs = &arg_engine_configs;

	status.f_is_init = true;
}

void WINDUP_Threading::deinit()
{
	std::vector<WINDUP_Thread> threads_to_join;
	{
		std::lock_guard<std::mutex> lock(mtx);
		threads_to_join = std::move(threads);
		threads.clear();
	}

	for (auto& thread : threads_to_join)
	{
		if (thread.handle.joinable())
		{
			thread.handle.join();
			WINDUP_Logger::info("Threading", "joined thread '" + thread.name + "'", 1);
		}
	}

	threads.clear();
	status.f_is_deinit = true;
}

bool WINDUP_Threading::spawn_thread(std::string name, std::function<void()> fn)
{
	std::lock_guard<std::mutex> lock(mtx);

	if (status.f_is_deinit)
	{
		WINDUP_Logger::error("Threading", "refused to spawn '" + name + "' engine is in deinitialization phase", 0);
		return false;
	}

	try
	{
		WINDUP_Thread thread;
		thread.name   = std::move(name);
		thread.handle = std::thread(std::move(fn));

		threads.push_back(std::move(thread));

		WINDUP_Logger::info("Threading", "spawned thread '" + thread.name + "'", 1);
		return true;
	}
	catch (const std::system_error &e)
	{
		WINDUP_Logger::error("Threading",
						 std::string("spawn_thread failed: ") + e.what(),
						 0);
		return false;
	}
}
