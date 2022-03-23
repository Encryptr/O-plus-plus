/** @file gc.h
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

#ifndef OPP_GC
#define OPP_GC

#include <stdio.h>
#include "../object/object.h"

struct GC_State {
    struct Opp_Obj* obj_list;
    struct Opp_Obj* gray_list;
    size_t used;
    size_t threshold;
    size_t growth_rate;
};

void opp_init_gc(struct GC_State* const gc);

#endif /* OPP_GC */