#include "csawstd.h"

#include <cstdarg>
#include <iostream>
#include <random>
#include <string>

double csaw::round(double x)
{
	return std::round(x);
}

double csaw::floor(double x)
{
	return std::floor(x);
}

double csaw::ceil(double x)
{
	return std::ceil(x);
}

double csaw::sin(double x)
{
	return std::sin(x);
}

double csaw::cos(double x)
{
	return std::cos(x);
}

double csaw::tan(double x)
{
	return std::tan(x);
}

double csaw::asin(double x)
{
	return std::asin(x);
}

double csaw::acos(double x)
{
	return std::acos(x);
}

double csaw::atan(double x)
{
	return std::atan(x);
}

double csaw::atan2(double y, double x)
{
	return std::atan2(y, x);
}

double csaw::random()
{
	return double(rand()) / (RAND_MAX + 1);
}

void csaw::printf(const char* format, ...)
{
	std::va_list va;
	va_start(va, format);
	vprintf(format, va);
	va_end(va);
}

char* csaw::readf(const char* format, ...)
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

double csaw::numFromStr(const char* x)
{
	return std::stod(x);
}

double csaw::numFromChr(char x)
{
	return double(x);
}

const char* csaw::strFromNum(double x)
{
	auto s = std::to_string(x);
	char* str = new char[s.length()];
	strcpy((char*)s.c_str(), str);
	return str;
}

char csaw::chrFromNum(double x)
{
	return char(x);
}

double csaw::streq(const char* a, const char* b)
{
	if (a == b)
		return 1;
	if (a == nullptr || b == nullptr)
		return 0;

	return strcmp(a, b) == 0;
}

double csaw::strlng(const char* x)
{
	return strlen(x);
}

char csaw::strat(const char* x, double i)
{
	return x[size_t(i)];
}
