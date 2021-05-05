/** @file util.c
 * 
 * @brief Opp utils
 *      
 * Copyright (c) 2021 Maks S
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
 */

#include "util.h"
#include "../opp.h"
#include "../memory/memory.h"
#include <setjmp.h>

void opp_error(struct Opp_Scan* s, const char* str, ...)
{
	va_list ap;
	va_start(ap, str);
	
	printf("%s:%u:%u ", s->io.fname, s->line, s->colum);
	#ifdef UNX 
	printf(CL_RED);
	#endif

	printf("error: \n\t");
	printf(CL_RESET); 
	vfprintf(stdout, str, ap);
	fprintf(stdout, "\n");
	va_end(ap);

	longjmp(global_state.error_buf, OPP_ERROR);
}

void internal_error(const char* str)
{
	#ifdef UNX 
	printf(CL_RED); 
	#endif 
	printf("** Error: %s **\n", str); 
	#ifdef UNX 
	printf(CL_RESET); 
	#endif 

	longjmp(global_state.error_buf, OPP_ERROR);
}

char* cpy_string(char* original) 
{
	char* cpy = (char*)opp_alloc(strlen(original) + 1);

	if (!cpy)
		MALLOC_FAIL();

	strcpy(cpy, original);

	return cpy;
}

unsigned int hash_str(char* string, unsigned int size)
{
	unsigned int h = 0;
	while (*string) {
		h = ((h<<5)-h)^(unsigned char)*(string++);
	}
	h = h % size;
	
	return h;
}

struct Opp_Hashmap* opp_create_map(size_t size, struct Opp_Hashmap* parent)
{
	struct Opp_Hashmap* map = (struct Opp_Hashmap*)
		opp_alloc(sizeof(struct Opp_Hashmap));

	if (!map)
		MALLOC_FAIL();

	map->items = (struct Opp_Bucket**)
		opp_alloc(sizeof(struct Opp_Bucket*)*size);

	map->size = size;
	map->parent = parent;
	
	memset(map->items, 0, sizeof(struct Opp_Bucket*)*size);

	return map;
}

struct Opp_Bucket* opp_get_bucket(struct Opp_Hashmap* map, char* string)
{
	unsigned int loc = hash_str(string, map->size);

	struct Opp_Hashmap* scope = map;

	while (scope != NULL)
	{
		if (scope->items[loc] != NULL) 
		{
			struct Opp_Bucket* node = scope->items[loc];

			while (node) {
				if (!strcmp(node->id, string)) 
					return node;
				else if (node->next != NULL)
					node = node->next;
				else 
					break;
			}
		}
		scope = scope->parent;
	}

	return NULL;
}

struct Opp_Bucket* opp_create_bucket(struct Opp_Hashmap* map, char* string)
{
	unsigned int loc = hash_str(string, map->size);

	if (map->items[loc] != NULL) {
		struct Opp_Bucket* pos = map->items[loc];
		while (pos) {
			if (!strcmp(pos->id, string)) 
				return NULL;
			else if (pos->next == NULL) {
				pos->next = (struct Opp_Bucket*)
					opp_alloc(sizeof(struct Opp_Bucket));

				if (!pos->next)
					MALLOC_FAIL();

				pos->next->id = cpy_string(string);
				pos->next->next = NULL;
				return pos->next;
			}
			else
				pos = pos->next;
		}
	}
	else {
		struct Opp_Bucket* node = (struct Opp_Bucket*)
			opp_alloc(sizeof(struct Opp_Bucket));

		if (!node)
			MALLOC_FAIL();

		node->id = cpy_string(string);
		node->next = NULL;
		map->items[loc] = node;

		return node;
	}

	return NULL;
}
