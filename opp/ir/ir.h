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
	ir->code.bytes[ir->code.idx++] = op

enum Regs {
	REG_RAX,
	REG_RCX,
	REG_RDX
};

enum OppIr_Opcode_Type {
	OPCODE_CONST,
	OPCODE_CALL,
	OPCODE_FUNC,
	OPCODE_VAR,
	OPCODE_ARITH,
	OPCODE_CMP,
	OPCODE_JMP,
	OPCODE_LABEL,
	OPCODE_DEREF,
	OPCODE_ADDR,
	OPCODE_ASSIGN,
	OPCODE_END,
};

struct Register {
	enum Regs reg;
	int32_t loc;
	bool used, spilled;
};

#define REG_COUNT 3
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
	IMM_STR, IMM_LOC, IMM_I64,
	IMM_U32, IMM_I32,
	IMM_I8, IMM_F64
};

struct OppIr_Const {
	enum OppIr_Const_Type type;
	bool global, nopush;
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
	unsigned int elem;
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

enum OppIr_Jmp_Type {
	PURE_JMP, RET_JMP,
	// Rest jmp types are in Opp_Token
};

struct OppIr_Jmp {
	enum OppIr_Jmp_Type type;
	unsigned int loc;
};

struct OppIr_Cmp {
	bool imm;
	struct OppIr_Const val;
};

struct OppIr_Set {
	bool global, imm;
	struct OppIr_Const val;
};

struct OppIr_Opcode {
	enum OppIr_Opcode_Type type;

	union {
		struct OppIr_Const constant;
		struct OppIr_Func  func;
		struct OppIr_Var   var;
		struct OppIr_Arith arith;
		struct OppIr_Jmp   jmp;
		struct OppIr_Cmp   cmp;
		struct OppIr_Set   set;
	};
};

struct OppIr_Instr {
	size_t instr_idx, allocated;
	struct OppIr_Opcode* opcodes;
};

struct Jmp_Item {
	size_t loc;
	unsigned int table_pos;
};

#define DEFAULT_OFFSET_TABLE 32
struct OppIr_Offsets {
	size_t allocated;
	unsigned int jmp_idx;
	int32_t* offset_table;
	struct Jmp_Item* jmp_table;
};

struct OppIr {
	struct Bytecode code;
	struct Allocation regalloc;
	struct Reg_Stack reg_stack;
	struct Stack local_stack;
	struct OppIr_Instr* instr;
	struct OppIr_Offsets offsets;
};

#ifdef LINUX64
#	include "ir-linux.h"
#endif

#ifdef MAC64
#	include "ir-mac.h"
#endif

struct OppIr* init_oppir();
void oppir_free(struct OppIr* ir);
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
static void oppir_set_offsets(struct OppIr* ir);

// Opcodes
static void oppir_eval_const(struct OppIr* ir, struct OppIr_Const* imm);
static void oppir_local_param(struct OppIr* ir, unsigned int args);
static void oppir_eval_func(struct OppIr* ir, struct OppIr_Func* fn);
static void oppir_eval_label(struct OppIr* ir, struct OppIr_Const* loc);
static void oppir_eval_jmp(struct OppIr* ir, struct OppIr_Jmp* jmp);
static void oppir_eval_var(struct OppIr* ir, struct OppIr_Var* var);
static void oppir_eval_cmp(struct OppIr* ir, struct OppIr_Cmp* cmp);
static void oppir_eval_set(struct OppIr* ir, struct OppIr_Set* set);
static void oppir_eval_end(struct OppIr* ir);

#endif /* OPP_IR */