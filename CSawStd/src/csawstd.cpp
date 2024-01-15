#include <csawstd/csawstd.h>

#include <iostream>
#include <random>
#include <string>

void csaw_vprintf(const char* format, std::va_list va)
{
	vprintf(format, va);
}

char* csaw_vreadf(const char* format, std::va_list va)
{
	vprintf(format, va);

	std::string line;
	std::getline(std::cin, line);

	char* str = new char[line.size() + 1];
	strcpy_s(str, line.length() + 1, line.c_str());

	return str;
}

void csaw_printf(const char* format, ...)
{
	std::va_list va;
	va_start(va, format);
	vprintf(format, va);
	va_end(va);
}

char* readf(const char* format, ...)
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

double random()
{
	return double(rand()) / (RAND_MAX + 1);
}

double str_to_num(const char* x)
{
	return std::stod(x);
}

double chr_to_num(char x)
{
	return double(x);
}

const char* num_to_str(double x)
{
	auto s = std::to_string(x);
	char* str = new char[s.length()];
	strcpy_s(str, s.length(), s.c_str());
	return str;
}

char num_to_chr(double x)
{
	return char(x);
}

double csaw_strcmp(const char* a, const char* b)
{
	return strcmp(a, b);
}

double csaw_strlen(const char* x)
{
	return double(strlen(x));
}

char csaw_strget(const char* x, double i)
{
	return x[size_t(i)];
}
