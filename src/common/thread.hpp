#pragma once

#include <thread>
#include <string>

struct WINDUP_Thread
{
	std::thread handle;
	std::string name;
};