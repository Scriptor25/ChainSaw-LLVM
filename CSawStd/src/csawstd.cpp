#include "csawstd.h"

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
	std::va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
}

char* csaw_readf(const char* format, ...)
{
	std::va_list valist;
	va_start(valist, format);
	auto str = csaw_vreadf(format, valist);
	va_end(valist);

	return str;
}

void csaw_vprintf(const char* format, va_list valist)
{
	vprintf(format, valist);
}

char* csaw_vreadf(const char* format, va_list valist)
{
	vprintf(format, valist);

	std::string line;
	std::getline(std::cin, line);

	char* str = new char[line.size() + 1];
	strcpy_s(str, line.length(), line.c_str());
	str[line.size()] = '\00';

	return str;
}

double csaw_str_to_num(const char* x)
{
	return std::stod(x);
}

double csaw_chr_to_num(char x)
{
	return double(x);
}

const char* csaw_num_to_str(double x)
{
	auto s = std::to_string(x);
	char* str = new char[s.length()];
	strcpy_s(str, s.length(), s.c_str());
	return str;
}

char csaw_num_to_chr(double x)
{
	return char(x);
}

double csaw_str_cmp(const char* a, const char* b)
{
	if (a == b)
		return 1;
	if (a == nullptr || b == nullptr)
		return 0;

	return strcmp(a, b);
}

double csaw_str_len(const char* x)
{
	return double(strlen(x));
}

char csaw_str_get(const char* x, double i)
{
	return x[size_t(i)];
}
