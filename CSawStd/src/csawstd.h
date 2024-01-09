#pragma once

#include <cstdarg>

extern "C"
{
	double csaw_round(double x);
	double csaw_floor(double x);
	double csaw_ceil(double x);

	double csaw_sin(double x);
	double csaw_cos(double x);
	double csaw_tan(double x);
	double csaw_asin(double x);
	double csaw_acos(double x);
	double csaw_atan(double x);
	double csaw_atan2(double y, double x);

	double csaw_random();

	void csaw_printf(const char* format, ...);
	char* csaw_readf(const char* format, ...);
	void csaw_vprintf(const char* format, va_list valist);
	char* csaw_vreadf(const char* format, va_list valist);

	double csaw_str_to_num(const char* x);
	double csaw_chr_to_num(char x);
	const char* csaw_num_to_str(double x);
	char csaw_num_to_chr(double x);

	double csaw_str_cmp(const char* a, const char* b);
	double csaw_str_len(const char* x);
	char csaw_str_get(const char* x, double i);
}
