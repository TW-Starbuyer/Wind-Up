#include "commands.hpp"


//--------------------------------------------------------------------------------
// Init / Deinit
//--------------------------------------------------------------------------------

void WINDUP_Commands::init(WINDUP_EngineContext &arg_annihilator)
{
	windup = &arg_annihilator;

	windup->modules.threading->spawn_thread("console_input", [this]()
	{
		std::string input;
		while (std::getline(std::cin, input))
		{
			if (!input.empty())
				execute(input);
		}
	});
}

void WINDUP_Commands::deinit()
{
	std::lock_guard<std::mutex> lock(mtx);
	commands_map.clear();
	entries.clear();
}

//--------------------------------------------------------------------------------
// Cmd Operations
//--------------------------------------------------------------------------------

bool WINDUP_Commands::register_immediate_cmd(const std::string &name, CmdFn fn)
{
	std::lock_guard<std::mutex> lock(mtx);
	commands_map[name] = std::move(fn);

	return true;
}

bool WINDUP_Commands::register_deferred_cmd(const std::string &name, CmdFn fn)
{
	std::lock_guard<std::mutex> lock(mtx);
	commands_map[name] = [this, fn = std::move(fn)](auto& args)
	{
		deferred_commands_queue.push_back(fn);
		return true;
	};
	return true;
}

bool WINDUP_Commands::flush_deferred_cmds()
{
	std::vector<CmdFn> cmds_to_run;
	{
		std::lock_guard<std::mutex> lock(mtx);
		std::swap(cmds_to_run, deferred_commands_queue);
	}

	for (auto &fn: cmds_to_run)
	{
		fn({});
	}

	return true;
}

bool WINDUP_Commands::execute(const std::string &input)
{
	auto args = parse(input);
	if (args.empty()) return "";

	std::string cmd_name = args[0];
	std::vector<std::string> cmd_args(args.begin() + 1, args.end());

	CmdFn fn;
	{
		std::unique_lock<std::mutex> lock(mtx);
		auto it = commands_map.find(cmd_name);
		if (it == commands_map.end())
		{
			lock.unlock();
			WINDUP_Logger::cmd_result("Console", "Unknown command: " + cmd_name, 0);
			return false;
		}
		fn = it->second;
	}

	bool result = fn(cmd_args);
	WINDUP_Logger::cmd_result("Console", "test", result);

	return result;
}


//--------------------------------------------------------------------------------
// Entry operations
//--------------------------------------------------------------------------------

std::vector<WINDUP_CommandEntry> WINDUP_Commands::get_entries()
{
	std::lock_guard<std::mutex> lock(mtx);
	return entries;
}

void WINDUP_Commands::clear_entries()
{
	std::lock_guard<std::mutex> lock(mtx);
	entries.clear();
}


//--------------------------------------------------------------------------------
// Parsing
//--------------------------------------------------------------------------------

std::vector<std::string> WINDUP_Commands::parse(const std::string &input)
{
	std::vector<std::string> args;
	std::string current;

	for (char c: input)
	{
		if (c == ' ')
		{
			if (!current.empty())
			{
				args.push_back(current);
				current.clear();
			}
		} else
		{
			current += c;
		}
	}

	if (!current.empty())
	{
		args.push_back(current);
	}

	return args;
}




