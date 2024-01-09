#pragma once

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

	double csaw_numFromStr(const char* x);
	double csaw_numFromChr(char x);
	const char* csaw_strFromNum(double x);
	char csaw_chrFromNum(double x);

	double csaw_streq(const char* a, const char* b);
	double csaw_strlng(const char* x);
	char csaw_strat(const char* x, double i);
}
