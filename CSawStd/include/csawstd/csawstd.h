#pragma once

#include <cstdarg>

extern "C" void csaw_vprintf(const char* format, std::va_list va);
extern "C" char* csaw_vreadf(const char* format, std::va_list va);
extern "C" void csaw_printf(const char* format, ...);
extern "C" char* readf(const char* format, ...);

extern "C" double random();

extern "C" double str_to_num(const char* x);
extern "C" double chr_to_num(char x);
extern "C" const char* num_to_str(double x);
extern "C" char num_to_chr(double x);

extern "C" double csaw_strcmp(const char* a, const char* b);
extern "C" double csaw_strlen(const char* x);
extern "C" char csaw_strget(const char* x, double i);
