/* @file memory.h
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

#ifndef OPP_ALLOCATOR
#define OPP_ALLOCATOR

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define BLOCK_SIZE 64000
#define ALLIGMENT 8

struct Block_Header {
    struct Block_Header *next;
    unsigned char *block, *free, *end;
};

struct Allocator {
    struct Block_Header* first;
    struct Block_Header* current;
    void* (*allocator)(size_t);
    void (*xfree)(void*);
};

bool allocator_init(void* (*xmalloc)(size_t), void (*xfree)(void*));
void* opp_alloc(size_t size);
void* opp_realloc(void* mem, size_t new_size, size_t old_size);
void allocator_free();
void allocator_reset();	

#endif /* OPP_ALLOCATOR */
