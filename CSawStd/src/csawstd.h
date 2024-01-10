#pragma once

#include <cstdarg>
#include <iostream>
#include <random>
#include <string>

/////////////////////////////////////
// TODO: implement custom varargs: //
//        (stolen from C)          //
//                                 //
// va_list = alloca ptr            //
// va_start(va_list)               //
//                                 //
// addr0 = load ptr from va_list   //
// elem0 = gep i8 from addr0 + 8   //
// store elem0 in va_list          //
// ?0 = load i32 from addr0        //
//                                 //
// addr1 = load ptr from va_list   //
// elem1 = gep i8 from addr1 + 8   //
// store elem1 in va_list		   //
// ?1 = load i32 from addr1		   //
// 								   //
// addr2 = load ptr from va_list   //
// elem2 = gep i8 from addr2 + 8   //
// store elem2 in va_list		   //
// ?2 = load i32 from addr2        //
//                                 //
// va_end(va_list)                 //
/////////////////////////////////////

extern "C" void csaw_printf(const char* format, std::va_list va)
{
	vprintf(format, va);
}

extern "C" char* csaw_readf(const char* format, std::va_list va)
{
	vprintf(format, va);

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
