/** @file vm.c
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

#include "vm.h"
#include "../memory/memory.h"
#include "../util/util.h"
#include "../parser/parser.h"

struct Opp_VM* opp_init_vm(struct Opp_Parser* const parser)
{
    // struct Opp_VM* vm = (struct Opp_VM*)opp_alloc(sizeof(struct Opp_VM*));

    // if (!vm)
    //     MALLOC_FAIL();

    // vm->parser = parser;

    // return vm;
}

void opp_dump_vm(const struct Opp_VM* const vm)
{
    // for (unsigned int i = 0; i < vm->parser->bytecode.len; i++) {
    //     const Instruction instr = vm->parser->bytecode.data[i];
    //     printf("[0x%x]\n\tOpcode: %d\n", instr, GET_OPCODE(instr));
    // }
}