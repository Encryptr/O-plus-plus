/** @file memory.c
 * 
 * @brief Memory pool implementation
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
 */ 

#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "../util/util.h"

static struct Allocator *root;

#ifdef DEBUG_ALLOCATOR
static size_t used_mem = 0;
#endif

static struct Block_Header* block_init()
{
    struct Block_Header *alloc = NULL;
    char *block = NULL;

    block = (char*)malloc(BLOCK_SIZE);
    alloc = (struct Block_Header*)
    	malloc(sizeof(struct Block_Header));

    if (alloc == NULL || block == NULL) 
    	return NULL;

    alloc->block = block;
    alloc->free = block;
    alloc->next = NULL;
    alloc->end = block + BLOCK_SIZE;

    return alloc;
}

bool allocator_init(void* (*xmalloc)(size_t), void (*xfree)(void*))
{
    root = (struct Allocator*)malloc(sizeof(struct Allocator));

    if (root == NULL) 
    	return false;

    root->first = block_init();

    if (root->first == NULL)
    	return false;

    root->current = root->first;
    #ifdef DEBUG_ALLOCATOR
    used_mem = 0;
    #endif

    return true;
}

void* opp_realloc(void* mem, size_t new_size, size_t old_size)
{
    void* buffer = opp_alloc(new_size);

    if (!buffer)
        return NULL;

    memcpy(buffer, mem, old_size);

    return buffer;
}

void* opp_alloc(size_t size)
{
    struct Block_Header* block = root->current;
    char *ptr = NULL;

    while ((size % ALLIGMENT) != 0)
    	size++;

    ptr = block->free;
    block->free += size;

    #ifdef DEBUG_ALLOCATOR
    used_mem += size;
    colored_printf(CL_GREEN, "Allocating: %ld Used: %ld\n", size, used_mem);
    #endif

    if (block->free >= block->end) {
        if (block->next != NULL) {
            block->next->free = block->next->block;
            root->current = block->next;
        }
        else {
            #ifdef DEBUG_ALLOCATOR
            colored_print(CL_GREEN, "Allocating extra block\n");
            #endif
        	block->next = block_init();

            if (block->next == NULL) 
            	return NULL;
            root->current = block->next;
        }

        ptr = root->current->free;
        root->current->free += size;
    }

    return ptr;
}

void allocator_reset()
{
    #ifdef DEBUG_ALLOCATOR
    colored_print(CL_GREEN, "Allocator reset\n");
    #endif
    root->current = root->first;
    root->current->free = root->current->block;
}

void allocator_free()
{
    struct Block_Header* ptr;
    struct Block_Header* hdr = root->first;

    while (hdr != NULL)
    {
        ptr = hdr->next;
        free(hdr->block);
        free(hdr);
        hdr = ptr;
    }
    free(root);
    root = NULL;
}
