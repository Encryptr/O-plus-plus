/* @file util.h
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

#ifndef OPP_UTIL
#define OPP_UTIL

#include "platform.h"
#include <assert.h>

#define INTERNAL_ERROR(str) \
    do { \
        opp_internal_error(__FILE__, __func__, __LINE__, str); \
    } while (0)

#define OPP_ASSERT(cond) assert(cond)

void opp_internal_error(const char* file,
                        const char* func,
                        const int line,
                        const char* str);

#define MALLOC_FAIL(cond) \
    if (cond) INTERNAL_ERROR("Memory allocation failed...")

#define colored_printf(color, str, ...) \
    opp_colored_print((void*)color, str, __VA_ARGS__)
#define colored_print(color, str) \
    opp_colored_print((void*)color, str)

void opp_colored_print(void* color, const char* str, ...);

struct Bucket {
	const char* id;
	void* data;
	struct Bucket* next;
};

struct Hashmap {
	size_t size;
	struct Bucket** items;
	struct Hashmap* parent;
};

struct Hashmap* init_map(size_t size, struct Hashmap* parent);
void free_map(struct Hashmap* map);
struct Bucket* get_bucket(struct Hashmap* map, char* string);
struct Bucket* create_bucket(struct Hashmap* map, char* string);

#endif /* OPP_UTIL */
