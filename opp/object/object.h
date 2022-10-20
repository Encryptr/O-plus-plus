/** @file object.h
 * 
 * @brief Opp Object / Value
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

#ifndef OPP_OBJ
#define OPP_OBJ

#include <stdint.h>
#include <stdbool.h>
#include "../vm/opcode.h"
#include "../util/array.h"

enum Opp_Obj_Type {
    OBJ_FUNC,
    OBJ_STRING,
    OBJ_ARRAY
};

struct Opp_Obj {
    enum Opp_Obj_Type type;
    struct Opp_Obj* next;
    bool marked;
};

struct Opp_Func_Obj {
    struct Opp_Obj obj;
    char* name;
    ARRAY(Instruction) instr;
    struct Opp_Value* stack_begin;
    struct Opp_Obj** constants;
    unsigned int consts_len;
};

struct Opp_String_Obj {
    struct Opp_Obj obj;
    char* str;
    unsigned int len;
};

enum Opp_Value_Type {
    VALUE_NULL,
    VALUE_BOOL,
    VALUE_FLOAT,
    VALUE_NUMBER,
    VALUE_OBJ
};

struct Opp_Value {
    enum Opp_Value_Type type;
    union {
        int64_t number;
        double  real;
        struct Opp_Obj* obj;
    } v;
};

#endif /* OPP_OBJ */