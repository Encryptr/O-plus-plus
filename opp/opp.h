/** @file opp.h
 * 
 * @brief Main opp header
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

#ifndef OPP_HEADER
#define OPP_HEADER

#include <setjmp.h>
#include "./memory/gc.h"

enum Opp_Flags {
    F_NONE     = 0,
    F_DEBUG    = 1 << 0,
    F_WARNINGS = 1 << 1,
};

enum Error_State {
    OPP_OK,
    OPP_ERROR
};

struct Opp_State {
    unsigned int flags;
    const char* fname;
    jmp_buf error_buf;
};

#define ERROR_RECOVER(state) \
    (enum Error_State)setjmp(state)

#define THROW_ERROR(state) \
    longjmp(state, OPP_ERROR)

void opp_init_module(struct Opp_State* const state);

#endif /* OPP_HEADER */