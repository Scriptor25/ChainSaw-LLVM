#include "csawstd.h"

#include <cstdarg>
#include <iostream>
#include <random>
#include <string>

double csaw_round(double x)
{
	return std::round(x);
}

double csaw_floor(double x)
{
	return std::floor(x);
}

double csaw_ceil(double x)
{
	return std::ceil(x);
}

double csaw_sin(double x)
{
	return std::sin(x);
}

double csaw_cos(double x)
{
	return std::cos(x);
}

double csaw_tan(double x)
{
	return std::tan(x);
}

double csaw_asin(double x)
{
	return std::asin(x);
}

double csaw_acos(double x)
{
	return std::acos(x);
}

double csaw_atan(double x)
{
	return std::atan(x);
}

double csaw_atan2(double y, double x)
{
	return std::atan2(y, x);
}

double csaw_random()
{
	return double(rand()) / (RAND_MAX + 1);
}

void csaw_printf(const char* format, ...)
{
	std::va_list va;
	va_start(va, format);
	vprintf(format, va);
	va_end(va);
}

char* csaw_readf(const char* format, ...)
{
	std::va_list va;
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

double csaw_numFromStr(const char* x)
{
	return std::stod(x);
}

double csaw_numFromChr(char x)
{
	return double(x);
}

const char* csaw_strFromNum(double x)
{
	auto s = std::to_string(x);
	char* str = new char[s.length()];
	strcpy_s(str, s.length(), s.c_str());
	return str;
}

char csaw_chrFromNum(double x)
{
	return char(x);
}

double csaw_streq(const char* a, const char* b)
{
	if (a == b)
		return 1;
	if (a == nullptr || b == nullptr)
		return 0;

	return strcmp(a, b) == 0;
}

double csaw_strlng(const char* x)
{
	return double(strlen(x));
}

char csaw_strat(const char* x, double i)
{
	return x[size_t(i)];
}
