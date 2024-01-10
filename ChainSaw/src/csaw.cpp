#include "csaw.h"
#include "parser/parser.h"

#include <iostream>

int main(int argc, const char** argv)
{
	std::string path = argv[0]; // exe path
	std::string filename;
	std::vector<std::string> args;
	std::vector<std::string> flags; // -...
	std::map<std::string, std::string> options; // --... ...

	std::string nextOption;

	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];

		if (!nextOption.empty())
		{
			options[nextOption] = arg;
			nextOption.clear();
			continue;
		}

		if (arg.find("--") == 0) // option
		{
			nextOption = arg.substr(2);
			continue;
		}

		if (arg.find("-") == 0) // flag
		{
			flags.push_back(arg.substr(1));
			continue;
		}

		if (filename.empty())
		{
			filename = arg;
			continue;
		}

		args.push_back(arg);
	}

	if (filename.empty())
	{
		csaw::Shell(path, flags, options);
		return 0;
	}

	return csaw::Run(path, filename, args, flags, options);
}

int csaw::Shell(
	const std::string& path,
	const std::vector<std::string>& flags,
	const std::map<std::string, std::string>& options)
{
	std::string input;

	Environment::InitEnvironment();
	auto env = std::make_shared<Environment>(path);

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
		csaw::Parse(env, stream);
	}

	return 0;
}

int csaw::Run(
	const std::string& path,
	const std::string& filename,
	const std::vector<std::string>& args,
	const std::vector<std::string>& flags,
	const std::map<std::string, std::string>& options)
{
	Environment::InitEnvironment();
	auto env = std::make_shared<Environment>(filename);

	if (!csaw::Parse(env, filename))
	{
		std::cerr << "undefined file name '" << filename << "'" << std::endl;
		return 1;
	}

	Environment::Module().setSourceFileName(filename);

	Environment::Module().print(llvm::outs(), nullptr);

	if (flags & "jit")
	{
		auto code = Environment::Run();
		std::cout << "Exit Code " << code << std::endl;
	}
	else
	{
		Environment::Compile("output.o");
	}

	return 0;
}

bool csaw::operator&(const std::vector<std::string> list, const std::string& element)
{
	return std::find(list.begin(), list.end(), element) != list.end();
}
