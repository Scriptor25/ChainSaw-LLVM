#pragma once

#include <cstdarg>

extern "C" void csaw_vprintf(const char* format, std::va_list va);
extern "C" char* csaw_vreadf(const char* format, std::va_list va);
extern "C" void csaw_printf(const char* format, ...);
extern "C" char* csaw_readf(const char* format, ...);

extern "C" double csaw_random();

extern "C" double csaw_str_to_num(const char* x);
extern "C" double csaw_chr_to_num(char x);
extern "C" const char* csaw_num_to_str(double x);
extern "C" char csaw_num_to_chr(double x);

extern "C" double csaw_str_cmp(const char* a, const char* b);
extern "C" double csaw_str_len(const char* x);
extern "C" char csaw_str_get(const char* x, double i);
