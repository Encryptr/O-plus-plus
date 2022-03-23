/** @file code.c
 * 
 * @brief Opp Bytecode
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
#include "../util/platform.h"
#include "../util/util.h"
#include "../memory/memory.h"

// void opp_init_bytecode(struct Bytecode* const b)
// {
//     b->instr = (Instruction*)
//         opp_alloc(sizeof(Instruction) * PARSER_BYTECODE_SIZE);

//     if (!b->instr)
//         MALLOC_FAIL();

//     memset(b->instr, 0, sizeof(Instruction) * PARSER_BYTECODE_SIZE);
//     b->allocated = PARSER_BYTECODE_SIZE;
//     b->len = 0;
// }

// void opp_add_opcode(struct Bytecode* const b, 
//                     enum Opp_Opcode opcode,
//                     int32_t operand,
//                     uint8_t extra)
// {
//     if (b->len >= b->allocated) {
//         INTERNAL_ERROR("REALLOC BYTECODE");
//     }

//     b->instr[b->len] |= opcode;
//     b->instr[b->len] |= (operand << 4);
//     // b->instr[b->len] |= extra;
//     b->len++;
// }