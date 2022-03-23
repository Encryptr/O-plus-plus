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

void* opp_os_alloc(size_t size)
{
    return OPP_OS_MALLOC(size);
}

void opp_os_free(void* ptr)
{
    OPP_OS_FREE(ptr);
}

void* opp_os_realloc(void* ptr, size_t new_size)
{
    return OPP_OS_REALLOC(ptr, new_size);
}

/* Opp general allocate function
    @Return - pointer to block if sucessful
    @Param gc - pointer to GC_State
    @Param old_size - old size of allocation
    @Param new_size > 0 - new size of allocation
*/
void* opp_realloc(struct GC_State* const gc, void* ptr, size_t old_size, size_t new_size)
{
    if (!gc) INTERNAL_ERROR("gc argument NULL");

#ifdef DEBUG_MEMORY
    colored_printf(CL_GREEN, "realloc (%p, %lu, %lu)\n", ptr, old_size, new_size);
#endif

    if (new_size == 0) {
        opp_os_free(ptr);
        return NULL;
    }

    void* buffer = opp_os_realloc(ptr, new_size);

    if (!buffer) {
        // collect gc 
        // buffer = 
        // retry
        INTERNAL_ERROR("Out of memory");
    }

    gc->used += new_size - old_size;

    return buffer;
}
