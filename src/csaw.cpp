#include "csaw.h"
#include "parser/parser.h"

#include <fstream>
#include <sstream>
#include <stdio.h>

int main(int argc, const char** argv)
{
	std::string exec = argv[0];
	if (argc == 1)
	{
		csaw::Shell(exec);
		return 0;
	}

	if (argv[1] == std::string("--help") || argv[1] == std::string("-h"))
	{
		std::cout << "shell: no args" << std::endl;
		std::cout << "run file: <filename> <args>..." << std::endl;
		return 0;
	}

	if (!csaw::Run(exec, argv[1], argc - 2, argv + 2))
		return 1;

	return 0;
}

void csaw::Shell(const std::string& exec)
{
	std::string input;
	while (true)
	{
		std::cout << ">> ";
		std::getline(std::cin, input);

		if (input == "exit")
			break;
		if (input == "clear")
		{
			std::cout << "\033\143";
			continue;
		}

		std::stringstream stream(input);
		csaw::Parse(stream);
	}
}

bool csaw::Run(const std::string& exec, const std::string& filename, int argc, const char** argv)
{
	std::ifstream stream(filename);
	if (!stream)
		return false;

	csaw::Parse(stream);
	return true;
}
