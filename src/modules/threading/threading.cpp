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
		switch (thread.retirement_option)
		{
			case WINDUP_ThreadRetirementOption::Detach:
				thread.handle.detach();
				WINDUP_Logger::info("Threading", "thread '" + thread.name + "' walked off the job (detached)", 1);
				break;

			case WINDUP_ThreadRetirementOption::Join:
				if (thread.handle.joinable())
				{
					thread.handle.join();
					WINDUP_Logger::info("Threading", "joined thread '" + thread.name + "'", 1);
				}
				break;
		}
	}

	threads.clear();
	status.f_is_deinit = true;
}

bool WINDUP_Threading::spawn_thread(std::string arg_name, WINDUP_ThreadRetirementOption arg_retirement_option, std::function<void()> fn)
{
	std::lock_guard<std::mutex> lock(mtx);

	if (status.f_is_deinit)
	{
		WINDUP_Logger::error("Threading", "refused to spawn '" + arg_name + "' engine is in deinitialization phase", 0);
		return false;
	}

	try
	{
		WINDUP_Thread thread{
			std::thread(std::move(fn)),
			std::move(arg_name),
			arg_retirement_option
		};

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
