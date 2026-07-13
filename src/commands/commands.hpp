#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <mutex>

#include "status.hpp"
#include "context.hpp"

#include "../modules/threading/threading.hpp"
#include "../logger/logger.hpp"

struct WINDUP_CommandEntry
{
	std::string input;
	std::string output;
};

class WINDUP_Commands
{
	public:
		using CmdFn = std::function<bool(const std::vector<std::string> & args)>;

		// Constructor/destructor
		WINDUP_Commands() = default;
		~WINDUP_Commands() = default;

		// Init/deinit
		void init(WINDUP_EngineContext &arg_annihilator);
		void deinit();

		// Cmd operations
		bool register_immediate_cmd(const std::string &name, CmdFn fn);
		bool register_deferred_cmd(const std::string &name, CmdFn fn);
		bool flush_deferred_cmds();
		bool execute(const std::string &input);

		// Entry operations
		std::vector<WINDUP_CommandEntry> get_entries();
		void clear_entries();

	private:
		// Ctx
		WINDUP_EngineContext *windup = nullptr;

		// Caches

		// Members
		std::mutex mtx;
		std::unordered_map<std::string, CmdFn> commands_map;
		std::vector<CmdFn> deferred_commands_queue;
		std::vector<WINDUP_CommandEntry> entries;

		// Functions
		std::vector<std::string> parse(const std::string &input);
};