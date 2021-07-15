/** @file code.h
 * 
 * @brief Opp VM Opcodes
 *      
 * Copyright (c) 2021 Maks S
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

#ifndef OPP_OPCODES
#define OPP_OPCODES

#include <inttypes.h>

typedef uint32_t Opcode; 

/*
	- Opcodes support full C types
	- Dont froget about bitfield types
	- Load will extend to unum or snum if integer
	  or f32 / f64 if floating point
	- Store will then cast to type given
	- Cast will first cast then extend back

	Opcodes are a 32 bit unsigned integer.

	Layout:

	1 Byte     1 Byte        2 byte
	[ OPCODE ] [ TYPE_INFO ] [ EXTRA ]

	Opcodes:
		OP_LOAD
		OP_LOAD_CONST
		OP_STORE
		OP_STORE_GLOBAL

		OP_ADD
		OP_SUB
		OP_MUL
		OP_DIV
		OP_MOD
		OP_NEG

		OP_SHL
		OP_SHR
		OP_OR
		OP_AND
		OP_XOR
		OP_BITNEG

		OP_EQ

	
	int a = 4;
	char b = a + 5;

	** REMEBER TO HANDLE STRUCTS / Unions

	LOAD_CONST 0 # 4
	STORE i32 0
	LOAD 0
	LOAD_CONST 1 # 5
	ADD
	STORE i8 1
*/

enum Opp_VM_Opcodes {
	OP_LOAD_CONST,
	OP_LOAD,
	OP_STORE,
	OP_STORE_GLOBAL,

	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_NEG,
};

#endif /* OPP_OPCODES */