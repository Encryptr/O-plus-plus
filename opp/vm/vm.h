/** @file vm.h
 * 
 * @brief Opp VM
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
**/

#ifndef OPP_VM
#define OPP_VM

#include "../object./object.h"
#include "opcode.h"

struct Func_Call_Frame {
    struct Opp_Obj_Func* func;
    Instruction* ip;
    struct Opp_Value* frame_stack;
};

struct Opp_VM {
    struct Opp_Parser* parser;
    struct Opp_Value* stack, *top;
    struct Func_Call_Frame* frames;
    unsigned int frame_size, frame_count;
};

struct Opp_VM* opp_init_vm(struct Opp_Parser* const parser);
void opp_dump_vm(const struct Opp_VM* const vm);

#endif /* OPP_VM */