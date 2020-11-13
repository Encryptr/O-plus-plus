/** @file ir.h
 * 
 * @brief Opp IR
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

#ifndef OPP_IR
#define OPP_IR

#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include "../header/os.h"
#include "../lexer/lexer.h"

#define INIT_BYTECODE_SIZE 64

#define IR_EMIT(op) \
	ir->code.bytes[ir->code.idx++] = op;

enum Regs {
	REG_RAX,
	REG_RCX,
	REG_RDX
};

enum OppIr_Opcode_Type {
	OPCODE_CONST,
	
	OPCODE_FUNC,

	OPCODE_VAR,
	OPCODE_ARITH,

	OPCODE_END,
};

struct Register {
	enum Regs reg;
	int32_t loc;
	bool used, spilled;
};

#define REG_COUNT 3
static struct Register regs[] = {
	{.reg = REG_RAX, .used = 0},
	{.reg = REG_RCX, .used = 0},
	{.reg = REG_RDX, .used = 0}
};

#define DEFAULT_REG_STACK 8
struct Reg_Stack {
	struct Register* stack;
	struct Register* top;
};

struct Stack {
	uint32_t size;
	int32_t pos;
	size_t frame_ptr;
};

#define DEFAULT_SPILL 8

struct Spill {
	bool use, made;
	int32_t loc;
};

struct Allocation {
	size_t amount, allocated;
	struct Spill* spills;
};

struct Bytecode {
	unsigned char* bytes;
	size_t idx, allocated;
};

enum OppIr_Const_Type {
	IMM_LOC, IMM_STR, IMM_I64,
	IMM_U32, IMM_I32,
	IMM_I8, IMM_F64
};

struct OppIr_Const {
	enum OppIr_Const_Type type;
	bool global;

	union {
		char*	 imm_sym;
		int64_t  imm_i64;
		uint32_t imm_u32;
		int32_t  imm_i32;
		char     imm_i8;
		double   imm_f64;
	};
};

struct OppIr_Var {
	bool global;
	char* name;
	uint32_t offset;
};

struct OppIr_Func {
	char* fn_name;
	bool private;
	unsigned int args;
};

enum OppIr_Arith_Type {
	ARITH_ADD,
	ARITH_SUB,
	ARITH_MUL,
	ARITH_DIV,
	ARITH_MOD,
};

struct OppIr_Arith {
	enum OppIr_Arith_Type type;
};

struct OppIr_Opcode {
	enum OppIr_Opcode_Type type;

	union {
		struct OppIr_Const constant;
		struct OppIr_Func  func;
		struct OppIr_Var   var;
		struct OppIr_Arith arith;
	};
};

struct OppIr_Instr {
	size_t instr_idx, allocated;
	struct OppIr_Opcode* opcodes;
};

struct OppIr {
	struct Bytecode code;
	struct Allocation regalloc;
	struct Reg_Stack reg_stack;
	struct Stack local_stack;
	struct OppIr_Instr* instr;
};

#ifdef LINUX64
#	include "ir-linux.h"
#endif

// ir-debug.c
// Ir debug tools
void oppir_pretty_print(struct OppIr* ir);

struct OppIr* init_oppir();
void oppir_get_opcodes(struct OppIr *ir, struct OppIr_Instr* instr);
void oppir_eval(struct OppIr* ir);
void oppir_eval_opcode(struct OppIr* ir, struct OppIr_Opcode* op);
void dump_bytes(struct OppIr* ir, OppIO* io);

// Util
static void oppir_check_realloc(struct OppIr* ir, unsigned int bytes);
static enum Regs oppir_push_reg(struct OppIr* ir);
static enum Regs oppir_pop_reg(struct OppIr* ir);
static enum Regs oppir_reg_alloc(struct OppIr* ir);
static void oppir_write_const(struct OppIr* ir, struct OppIr_Const* imm);
static int32_t oppir_get_spill(struct OppIr* ir);

// Opcodes
static void oppir_eval_const(struct OppIr* ir, struct OppIr_Const* imm);
static void oppir_local_param(struct OppIr* ir, unsigned int args);
static void oppir_eval_func(struct OppIr* ir, struct OppIr_Func* fn);
static void oppir_eval_end(struct OppIr* ir);

#endif /* OPP_IR */