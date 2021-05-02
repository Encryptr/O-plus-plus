/** @file util.h
 * 
 * @brief Opp Utilities
 *      
 * Copyright (c) 2020 Maks S
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

#ifndef OPP_UTIL
#define OPP_UTIL

#include <stdio.h>
#include <stdlib.h>
#include "../lexer/lexer.h"

#ifdef MINGW
#	define WIN64
#endif

#ifdef __APPLE__
#	define MAC64
#	define UNX
#endif

#ifdef __linux__
#	define LINUX64
#	define UNX
#endif

#ifdef __x86_64__
#	define X86_CPU
#endif

#if !defined(WIN) && !defined(UNX)
	#define OS_ERROR
#endif

#ifdef UNX
	#define CL_RED    "\x1b[31m"
	#define CL_BLUE   "\x1b[34m"
	#define CL_GREEN  "\x1b[32m"
	#define CL_RESET  "\x1b[0m"
	#define CL_YELLOW "\x1b[33m"
#endif

#define INTERNAL_ERROR(str) \
	internal_error(str)
#define MALLOC_FAIL() \
	INTERNAL_ERROR("Malloc fail")

// Error Util
void opp_error(struct Opp_Scan* s, const char* str, ...);
void internal_error(const char* str);

// Hashmap
struct Opp_Bucket {
	const char* id;
	void* data;
	struct Opp_Bucket* next;
};

struct Opp_Hashmap {
	size_t size;
	struct Opp_Bucket** items;
	struct Opp_Hashmap* parent;
};

unsigned int hash_str(char* string, unsigned int size);
struct Opp_Hashmap* opp_create_map(size_t size, struct Opp_Hashmap* parent);
struct Opp_Bucket* opp_get_bucket(struct Opp_Hashmap* map, char* string);
struct Opp_Bucket* opp_create_bucket(struct Opp_Hashmap* map, char* string);
char* cpy_string(char* original);

#endif /* OPP_UTIL */
