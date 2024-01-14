#pragma once

#include <map>
#include <string>
#include <vector>

namespace csaw
{
	int Shell(
		const std::string& path,
		const std::vector<std::string>& flags,
		const std::map<std::string, std::string>& options);
	int Run(
		const std::string& path,
		const std::string& filename,
		const std::vector<std::string>& args,
		const std::vector<std::string>& flags,
		const std::map<std::string, std::string>& options);

	inline bool operator&(const std::vector<std::string> list, const std::string& element);
}