#pragma once
#include <string>
#include <iostream>

namespace verbose
{
	// A global flag to enable or disable verbose output
	extern bool enable;

	// Write a verbose message to standard output if enabled
	static void write(std::string msg)
	{
		if (enable) std::cout << msg << std::endl;
	}
}
