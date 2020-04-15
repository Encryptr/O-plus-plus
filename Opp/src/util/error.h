#ifndef OLISP_ERROR
#define OLISP_ERROR

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "../lexer/lexer.h"
#include "../opp.h"

#ifdef UNX
	#define CL_RED    "\x1b[31m"
	#define CL_BLUE   "\x1b[34m"
	#define CL_GREEN  "\x1b[32m"
	#define CL_RESET  "\x1b[0m"
	#define CL_YELLOW "\x1b[33m"
#endif

void opp_error(struct Opp_Scan* s, const char* str, ...);

/* Error Codes
1 - Malloc fail
2 - Fatal Error
*/
void internal_error(const char* str, int code);

#endif