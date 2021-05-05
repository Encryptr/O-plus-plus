/** @file memory.c
 * 
 * @brief Memory pool implementation
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

#include "memory.h"

static struct Allocator root;

static struct Block_Header* block_init()
{
    struct Block_Header *alloc;

    alloc = (struct Block_Header*)
        root.allocator(sizeof(struct Block_Header));

    if (!alloc)
        return NULL;

    alloc->block = (unsigned char*)malloc(BLOCK_SIZE);

    if (!alloc->block) 
    	return NULL;

    alloc->free = alloc->block;
    alloc->next = NULL;
    alloc->end = alloc->block + BLOCK_SIZE;

    return alloc;
}

bool allocator_init(void* (*xmalloc)(size_t), void (*xfree)(void*))
{
    root.allocator = xmalloc;
    root.xfree = xfree;
    root.first = block_init();

    if (root.first == NULL)
    	return false;

    root.current = root.first;

    return true;
}

void* opp_alloc(size_t size)
{
    struct Block_Header* block = root.current;
    unsigned char *ptr = NULL;

    while ((size % ALLIGMENT) != 0)
    	size++;

    ptr = block->free;
    block->free += size;

    if (block->free >= block->end) {
        if (block->next != NULL) {
            block->next->free = block->next->block;
            root.current = block->next;
        }
        else {
        	block->next = block_init();

            if (block->next == NULL) 
            	return NULL;

            root.current = block->next;
        }

        ptr = root.current->free;
        root.current->free += size;
    }

    return ptr;
}

void* opp_realloc(void* mem, size_t new_size, size_t old_size)
{
    void* buffer = opp_alloc(new_size);

    if (!buffer)
        return NULL;

    memcpy(buffer, mem, old_size);

    return buffer;
}

void allocator_reset()
{
    root.current = root.first;
    root.current->free = root.current->block;
}

void allocator_free()
{
    struct Block_Header* ptr;
    struct Block_Header* hdr = root.first;

    while (hdr != NULL)
    {
        ptr = hdr->next;
        root.xfree(hdr->block);
        root.xfree(hdr);
        hdr = ptr;
    }
}
