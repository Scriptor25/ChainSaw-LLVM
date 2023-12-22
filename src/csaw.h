#pragma once

#include <string>

namespace csaw
{
	void Shell(const std::string& exec);
	bool Run(const std::string& exec, const std::string& filename, int argc, const char** argv);
}