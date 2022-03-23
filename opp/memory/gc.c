/** @file gc.c
 * 
 * @brief Garbage Collector implementation
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

#include "gc.h"
#include "../util/platform.h"

void opp_init_gc(struct GC_State* const gc)
{
    gc->used = 0;
    gc->threshold = OPP_INITIAL_HEAP_SIZE;
    gc->growth_rate = 2;
}