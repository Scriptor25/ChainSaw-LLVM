#include <csaw/csaw.h>
#include <csaw/parser.h>

#include <iostream>
#include <sstream>

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
	std::string filename;
	std::vector<std::string> args;
	auto env = std::make_shared<Environment>(path, filename, args, flags, options);

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
		Parse(env, stream);

		auto result = env->RunJIT();
		std::cout << "Exit Code " << result << std::endl;
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
	auto env = std::make_shared<Environment>(path, filename, args, flags, options);
	if (!Parse(env, filename))
	{
		std::cerr << "Failed to parse file '" << filename << "'" << std::endl;
		return 1;
	}

	env->Module().print(llvm::outs(), nullptr);

	std::cout << "Running JIT..." << std::endl;
	auto result = env->RunJIT();
	std::cout << "Exit Code " << result << std::endl;

	return 0;
}

bool csaw::operator&(const std::vector<std::string> list, const std::string& element)
{
	return std::find(list.begin(), list.end(), element) != list.end();
}
