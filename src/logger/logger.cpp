#include "logger.hpp"


std::vector<WINDUP_LogEntry> WINDUP_Logger::entries;
std::mutex WINDUP_Logger::mtx;
size_t WINDUP_Logger::max_log_entries;
size_t WINDUP_Logger::print_cursor;

//--------------------------------------------------------------------------------
// Init / Deinit
//--------------------------------------------------------------------------------

void WINDUP_Logger::init()
{
	max_log_entries = configs.max_log_entries;
	print_cursor = 0;
}

void WINDUP_Logger::deinit()
{

}

//--------------------------------------------------------------------------------
// Log operations
//--------------------------------------------------------------------------------

void WINDUP_Logger::cmd_result(const std::string &category, const std::string &msg, int success)
{
	log(WINDUP_LogLevel::CmdResult, category, msg, success);
}

void WINDUP_Logger::info(const std::string &category, const std::string &msg, int success)
{
	log(WINDUP_LogLevel::Info, category, msg, success);
}

void WINDUP_Logger::task_result(const std::string &category, const std::string &msg, int success)
{
	log(WINDUP_LogLevel::TaskResult, category, msg, success);
}

void WINDUP_Logger::warning(const std::string &category, const std::string &msg, int success)
{
	log(WINDUP_LogLevel::Warning, category, msg, success);
}

void WINDUP_Logger::error(const std::string &category, const std::string &msg, int success)
{
	log(WINDUP_LogLevel::Error, category, msg, success);
}

void WINDUP_Logger::log(WINDUP_LogLevel level, const std::string& category, const std::string& msg, int success)
{
	std::lock_guard<std::mutex> lock(mtx);
	entries.push_back({level, category, msg});
	if (entries.size() > max_log_entries)
	{
		entries.erase(entries.begin());
		if (print_cursor > 0) --print_cursor;
	}
}


//--------------------------------------------------------------------------------
// Entry operations
//--------------------------------------------------------------------------------

std::vector<WINDUP_LogEntry> WINDUP_Logger::get_entries()
{
	std::lock_guard<std::mutex> lock(mtx);
	return entries;
}

void WINDUP_Logger::clear_entries()
{
	std::lock_guard<std::mutex> lock(mtx);
	entries.clear();
}

void WINDUP_Logger::print_entries()
{
	std::lock_guard<std::mutex> lock(mtx);
	for (size_t i = print_cursor; i < entries.size(); ++i)
	{
		auto& log_entry = entries[i];
		bool print = false;
		switch (log_entry.level)
		{
			case WINDUP_LogLevel::CmdResult: print = configs.print_level_cmd_result; break;
			case WINDUP_LogLevel::Info:      print = configs.print_level_info;        break;
			case WINDUP_LogLevel::TaskResult:print = configs.print_level_task_result; break;
			case WINDUP_LogLevel::Warning:   print = configs.print_level_warning;     break;
			case WINDUP_LogLevel::Error:     print = configs.print_level_error;       break;
			default: break;
		}
		if (print)
			std::cout << "[" << to_string(log_entry.level) << "] "
					  << log_entry.category << ": " << log_entry.message << "\n";
	}
	print_cursor = entries.size();
}

//--------------------------------------------------------------------------------
// Conversions
//--------------------------------------------------------------------------------

std::string_view WINDUP_Logger::to_string(WINDUP_LogLevel level)
{
	switch (level)
	{
		case WINDUP_LogLevel::CmdResult: return "CMD RESULT";
		case WINDUP_LogLevel::Info: return "INFO";
		case WINDUP_LogLevel::TaskResult: return "TASK RESULT";
		case WINDUP_LogLevel::Warning: return "WARNING";
		case WINDUP_LogLevel::Error: return "ERROR";
		default: return "UNKNOWN";
	}
}