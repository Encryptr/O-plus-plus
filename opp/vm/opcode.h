/** @file opcode.c
 * 
 * @brief Opp Opcodes
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

#ifndef OPP_OPCODE
#define OPP_OPCODE

#include <stdint.h>

/*
    Opcode: usigned 32 bit 

    [ 8 bits ] (T)
        Opcode type

    [ 32 bits ] (O)
        Operand

    [ 8 bits ] (E)
        Extra
*/

typedef uint32_t Instruction;

#define GET_OPCODE(instr) (uint8_t)(instr & 0xF)

enum Opp_Opcode {

    OP_END,

    /* Push null value to stack */
    OP_PUSH_NULL,
    
    OP_PUSH_VALUE,
    OP_PUSH_TRUE,
    OP_PUSH_FALSE,
    

};

// void opp_init_bytecode(struct Bytecode* const b);
// void opp_add_opcode(struct Bytecode* const b, 
//                     enum Opp_Opcode opcode,
//                     int32_t operand,
//                     uint8_t extra);

#endif /* OPP_OPCODE */
