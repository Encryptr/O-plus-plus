/** @file env.c
 * 
 * @brief Opp compiler enviroment
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
 */

#include "env.h"

unsigned int hash_str(char *string, unsigned int size)
{
	unsigned int h = 0;
	while (*string) {
		h = ((h<<5)-h)^(unsigned char)*(string++);
	}
	h = h % size;
	
	return h;
}

struct Opp_Namespace* init_namespace(struct Opp_Namespace* parent, void* (*xmalloc)(size_t))
{
	struct Opp_Namespace* ns = (struct Opp_Namespace*)
		xmalloc(sizeof(struct Opp_Namespace));

	if (ns == NULL) 
		INTERNAL_ERROR("Malloc Fail");

	ns->xmalloc = xmalloc;
	ns->parent = parent;
	ns->items = (struct Opp_Bucket**)
		xmalloc(sizeof(struct Opp_Bucket*)*HASH_SIZE);

	if (ns->items == NULL)
		INTERNAL_ERROR("Malloc Fail");

	ns->size = HASH_SIZE;
	
	for (int i = 0; i < HASH_SIZE; i++)
		ns->items[i] = NULL;

	return ns;
}

struct Opp_Bucket* env_add_item(struct Opp_Namespace* ns, char* name)
{
	unsigned int loc = hash_str(name, ns->size);

	if (ns->items[loc] != NULL) {
		struct Opp_Bucket* pos = ns->items[loc];

		while (pos) {
			if (!strcmp(pos->key, name)) 
				return NULL;
			else if (pos->next == NULL) {
				pos->next = (struct Opp_Bucket*)
					ns->xmalloc(sizeof(struct Opp_Bucket));

				if (!pos->next)
					INTERNAL_ERROR("Malloc fail");

				pos->next->key = name;
				return pos;
			}
			else 
				pos = pos->next;
		}
	}
	else {
		struct Opp_Bucket* node = (struct Opp_Bucket*)
			ns->xmalloc(sizeof(struct Opp_Bucket));

		if (node == NULL)
			INTERNAL_ERROR("Malloc fail");

		node->key = name;
		node->next = NULL;
		ns->items[loc] = node;

		return node;
	}

	return NULL;
}

struct Opp_Bucket* env_get_item(struct Opp_Namespace* ns, char* name)
{
	unsigned int loc = hash_str(name, ns->size);

	struct Opp_Namespace* scope = ns;

	while (scope != NULL)
	{
		if (scope->items[loc] != NULL) 
		{
			struct Opp_Bucket* node = scope->items[loc];

			while (node) {
				if (!strcmp(node->key, name)) 
					return node;
				else if (node->next != NULL)
					node = node->next;
				else 
					return NULL;

			}
		}
		scope = scope->parent;
	}

	return NULL;
}

void env_free(struct Opp_Namespace* ns)
{
	for (size_t i = 0; i < ns->size; i++) {
		
	}
}