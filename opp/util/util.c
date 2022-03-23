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
#include "../memory/memory.h"

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

// static unsigned int hash_str(char* string, unsigned int size)
// {
// 	unsigned int h = 0;
// 	while (*string) {
// 		h = ((h<<5)-h)^(unsigned char)*(string++);
// 	}
// 	h = h % size;
	
// 	return h;
// }

// struct Hashmap* opp_create_map(size_t size, struct Hashmap* parent)
// {
// 	struct Hashmap* map = (struct Hashmap*)
// 		opp_alloc(sizeof(struct Hashmap));

// 	if (!map)
// 		MALLOC_FAIL();

// 	map->items = (struct Bucket**)
// 		opp_alloc(sizeof(struct Bucket*)*size);
    
//     if (!map->items)
//         MALLOC_FAIL();

// 	map->size = size;
// 	map->parent = parent;
	
// 	memset(map->items, 0, sizeof(struct Bucket*)*size);

// 	return map;
// }

// struct Bucket* opp_get_bucket(struct Hashmap* map, char* string)
// {
// 	unsigned int loc = hash_str(string, map->size);

// 	struct Hashmap* scope = map;

// 	while (scope != NULL)
// 	{
// 		if (scope->items[loc] != NULL) 
// 		{
// 			struct Bucket* node = scope->items[loc];

// 			while (node) {
// 				if (!strcmp(node->id, string)) 
// 					return node;
// 				else if (node->next != NULL)
// 					node = node->next;
// 				else 
// 					break;
// 			}
// 		}
// 		scope = scope->parent;
// 	}

// 	return NULL;
// }

// struct Bucket* opp_create_bucket(struct Hashmap* map, char* string)
// {
// 	unsigned int loc = hash_str(string, map->size);

// 	if (map->items[loc] != NULL) {
// 		struct Bucket* pos = map->items[loc];
// 		while (pos) {
// 			if (!strcmp(pos->id, string)) 
// 				return NULL;
// 			else if (pos->next == NULL) {
// 				pos->next = (struct Bucket*)
// 					opp_alloc(sizeof(struct Bucket));

// 				if (!pos->next)
// 					MALLOC_FAIL();

// 				pos->next->id = string;
// 				pos->next->next = NULL;
// 				return pos->next;
// 			}
// 			else
// 				pos = pos->next;
// 		}
// 	}
// 	else {
// 		struct Bucket* node = (struct Bucket*)
// 			opp_alloc(sizeof(struct Bucket));

// 		if (!node)
// 			MALLOC_FAIL();

// 		node->id = string;
// 		node->next = NULL;
// 		map->items[loc] = node;

// 		return node;
// 	}

// 	return NULL;
// }