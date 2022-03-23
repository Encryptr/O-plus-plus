/** @file memory.h
 * 
 * @brief Memory implementation
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

#ifndef OPP_MEM
#define OPP_MEM

#include <stdio.h>
#include "gc.h"

void* opp_os_alloc(size_t size);
void  opp_os_free(void* ptr);
void* opp_os_realloc(void* ptr, size_t new_size);
void* opp_realloc(struct GC_State* const gc, void* ptr, size_t old_size, size_t new_size);

#define OPP_ALLOC(gc, size) \
    opp_realloc(gc, NULL, 0, size)

#define OPP_FREE(gc, ptr) \
    opp_realloc(gc, ptr, 0, 0);

#endif /* OPP_MEM */