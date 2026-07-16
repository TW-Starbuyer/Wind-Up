#pragma once

// STL INCLUDES
#include <iostream>
#include <string>
#include <vector>
#include <mutex>

// THIRD-PARTY INCLUDES
#include <glm/glm.hpp>

// COMMON INCLUDES
#include "configs.hpp"

//----------------------------------------------------------------------------------------------

enum class WINDUP_LogLevel { CmdResult, Info, TaskResult, Warning, Error };

struct WINDUP_LogEntry
{
	WINDUP_LogLevel level;
	std::string category;
	std::string message;
	int success = -1;
};

class WINDUP_Logger
{
public:
	inline static WINDUP_LoggerConfigs configs{};

	WINDUP_Logger() = default;
	~WINDUP_Logger() = default;

	static void init();
	static void deinit();

	static void cmd_result(const std::string &category, const std::string &msg, int success);
	static void task_result(const std::string &category, const std::string &msg, int success);
	static void info(const std::string &category, const std::string &msg, int success);
	static void warning(const std::string &category, const std::string &msg, int success);
	static void error(const std::string &category, const std::string &msg, int success);

	static std::vector<WINDUP_LogEntry> get_entries();
	static void clear_entries();
	static void print_entries();

	static std::string_view to_string(WINDUP_LogLevel level);

private:
	static size_t max_log_entries;
	static size_t print_cursor;
	static std::mutex mtx;
	static std::vector<WINDUP_LogEntry> entries;

	static void log(WINDUP_LogLevel level, const std::string &category, const std::string &msg, int success);


};