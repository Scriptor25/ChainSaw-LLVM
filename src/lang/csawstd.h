#pragma once

#include <iostream>
#include <random>
#include <string>
#include <varargs.h>

extern "C"
{
	static char* readf(const char* format, ...)
	{
		va_list va;
		va_start(va, format);
		vprintf(format, va);
		va_end(va);

		std::string line;
		std::getline(std::cin, line);

		char* value = new char[line.size() + 1];
		for (size_t i = 0; i < line.size(); i++)
			value[i] = line[i];
		value[line.size()] = '\00';

		return value;
	}

	static double num(char* str)
	{
		return std::stod(str);
	}

	static double random()
	{
		return double(rand()) / (RAND_MAX + 1);
	}
}
