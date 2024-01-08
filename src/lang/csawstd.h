#pragma once

namespace csaw
{
	double round(double x);
	double floor(double x);
	double ceil(double x);

	double sin(double x);
	double cos(double x);
	double tan(double x);
	double asin(double x);
	double acos(double x);
	double atan(double x);
	double atan2(double y, double x);

	double random();

	void printf(const char* format, ...);
	char* readf(const char* format, ...);

	double numFromStr(const char* x);
	double numFromChr(char x);
	const char* strFromNum(double x);
	char chrFromNum(double x);

	double streq(const char* a, const char* b);
	double strlng(const char* x);
	char strat(const char* x, double i);
}
