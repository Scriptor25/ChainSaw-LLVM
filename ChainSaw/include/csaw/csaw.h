#pragma once

#include <map>
#include <string>
#include <vector>

namespace csaw
{
	int Shell(
		std::string& path,
		std::vector<std::string>& flags,
		std::map<std::string, std::string>& options);
	int Run(
		std::string& path,
		std::string& filename,
		std::vector<std::string>& args,
		std::vector<std::string>& flags,
		std::map<std::string, std::string>& options);

	inline bool operator&(const std::vector<std::string>& list, const std::string& element);
}