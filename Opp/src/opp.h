#ifndef OLISP
#define OLISP

#if defined(MINGW) || defined(__MINGW32__)
	#define WIN
#endif

#ifdef __APPLE__
	#define UNX
#endif

#ifdef __linux__
	#define UNX
	#define LNX
#endif

#if !defined(WIN) && !defined(UNX)
	#define OS_ERROR
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "./lexer/lexer.h"

void init_file(const char* fname, struct Opp_Scan *s);
void init_opp(const char* fname);

#endif