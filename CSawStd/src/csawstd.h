#pragma once

#include <cstdarg>
#include <iostream>
#include <random>
#include <string>

extern "C" void csaw_vprintf(const char* format, std::va_list va)
{
	vprintf(format, va);
}

extern "C" char* csaw_vreadf(const char* format, std::va_list va)
{
	vprintf(format, va);

	std::string line;
	std::getline(std::cin, line);

	char* str = new char[line.size() + 1];
	strcpy_s(str, line.length() + 1, line.c_str());

	return str;
}

extern "C" void csaw_printf(const char* format, ...)
{
	std::va_list va;
	va_start(va, format);
	vprintf(format, va);
	va_end(va);
}

extern "C" char* csaw_readf(const char* format, ...)
{
	std::va_list va;
	va_start(va, format);
	vprintf(format, va);
	va_end(va);

	std::string line;
	std::getline(std::cin, line);

	char* str = new char[line.size() + 1];
	strcpy_s(str, line.length() + 1, line.c_str());

	return str;
}

extern "C" double csaw_random()
{
	return double(rand()) / (RAND_MAX + 1);
}

extern "C" double csaw_str_to_num(const char* x)
{
	return std::stod(x);
}

extern "C" double csaw_chr_to_num(char x)
{
	return double(x);
}

extern "C" const char* csaw_num_to_str(double x)
{
	auto s = std::to_string(x);
	char* str = new char[s.length()];
	strcpy_s(str, s.length(), s.c_str());
	return str;
}

extern "C" char csaw_num_to_chr(double x)
{
	return char(x);
}

extern "C" double csaw_str_cmp(const char* a, const char* b)
{
	if (a == b)
		return 1;
	if (!a || !b)
		return 0;

	return strcmp(a, b);
}

extern "C" double csaw_str_len(const char* x)
{
	return double(strlen(x));
}

extern "C" char csaw_str_get(const char* x, double i)
{
	return x[size_t(i)];
}
