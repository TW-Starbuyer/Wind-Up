#include "threading.hpp"

void WINDUP_Threading::init(WINDUP_EngineConfigs& arg_engine_configs)
{
	engine_configs = &arg_engine_configs;

	status.f_is_init = true;
}

void WINDUP_Threading::deinit()
{
	std::lock_guard<std::mutex> lock(mtx);

	for (auto& t : threads)
	{
		if (t.handle.joinable())
		{
			t.handle.join();
			WINDUP_Logger::info("Threading", "joined thread '" + t.name + "'", 1);
		}
	}

	threads.clear();
	status.f_is_deinit = true;
}

bool WINDUP_Threading::spawn_thread(std::string name, std::function<void()> fn)
{
	std::lock_guard<std::mutex> lock(mtx);

	try
	{
		WINDUP_Thread t;
		t.name   = std::move(name);
		t.handle = std::thread(std::move(fn));

		threads.push_back(std::move(t));

		WINDUP_Logger::info("Threading", "spawned thread '" + t.name + "'", 1);
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
