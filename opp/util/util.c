/** @file util.c
 * 
 * @brief Opp Utilities
 *      
 * Copyright (c) 2022 Maks S
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#include "util.h"
#include <stdio.h>
#include <stdlib.h>

void opp_internal_error(const char* file,
                        const char* func,
                        const int line,
                        const char* str)
{
    // Add colors
    fprintf(stdout, "[ Internal Error ] %s:%s:%d\n\t%s\n",
        file, func, line, str);

    exit(EXIT_FAILURE);
}

void opp_colored_print(void* color, const char* str, ...)
{
    va_list ap;
	va_start(ap, str);

    #if defined(SYSTEM_WIN_64)
    HANDLE hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, (INT_PTR)color);
    #elif defined(SYSTEM_UNX)
    fprintf(stdout, color);
    #endif

	vfprintf(stdout, str, ap);

    #if defined(SYSTEM_WIN_64)
    SetConsoleTextAttribute(hConsole, CL_RESET);
    #elif defined(SYSTEM_UNX)
    fprintf(stdout, "%s", CL_RESET);
    #endif

	va_end(ap);
}
