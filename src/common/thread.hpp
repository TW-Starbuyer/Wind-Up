#pragma once

#include <thread>
#include <string>

// 401k match, health care, and golf club membership included.
enum class WINDUP_ThreadRetirementOption
{
	Join,
	Detach
};

struct WINDUP_Thread
{
	std::thread handle;
	std::string name;
	WINDUP_ThreadRetirementOption retirement_option;
};