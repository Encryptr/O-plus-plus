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

#define BLOCK_REG(reg) \
	regs[reg].used = 1

#define UNBLOCK_REG(reg) \
	regs[reg].used = 0

enum Regs {
	REG_RAX, REG_RCX, REG_RDX,

	REG_XMM0, REG_XMM1, REG_XMM2
};

enum OppIr_Opcode_Type {
	OPCODE_CONST,
	OPCODE_CALL,
	OPCODE_FUNC,
	OPCODE_ARG,
	OPCODE_VAR,
	OPCODE_ARITH,
	OPCODE_CMP,
	OPCODE_JMP,
	OPCODE_RET,
	OPCODE_LABEL,
	OPCODE_CAST,
	OPCODE_DEREF,
	OPCODE_BIT,
	OPCODE_ADDR,
	OPCODE_ASSIGN,
	OPCODE_END,
};

enum OppIr_Const_Type {
	IMM_STR, IMM_LOC,
	
	IMM_U8,  IMM_I8, 
	IMM_U16, IMM_I16,
	IMM_U32, IMM_I32,
	IMM_U64, IMM_I64,

	IMM_F32, IMM_F64
};

struct Register {
	enum Regs reg;
	enum OppIr_Const_Type type;
	int32_t loc;
	bool used, spilled;
};

#define REG_COUNT 6
#define DEFAULT_REG_STACK 16

struct Reg_Stack {
	size_t allocated;
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

struct OppIr_Value {
	enum OppIr_Const_Type type;

	union {
		char*	  imm_sym;
		int64_t   imm_i64;
		int32_t   imm_i32;
		int8_t    imm_i8;
		double    imm_f64;
		float	  imm_f32;
	};
};

struct OppIr_Const {
	struct OppIr_Value val;
	enum OppIr_Const_Type loc_type;
	bool global, nopush;
};

struct OppIr_Var {
	bool global;
	char* name;
	unsigned int size;
};

struct OppIr_Func {
	char* fn_name;
	bool private;
	unsigned int sym;
};

struct OppIr_Arg {
	enum OppIr_Const_Type type;
	unsigned int idx;
};

enum {
	PURE_JMP = 0,
};

struct OppIr_Jmp {
	int type;
	unsigned int loc;
};

struct OppIr_Cmp {
	bool imm;
	struct OppIr_Const val;
};

struct OppIr_Set {
	struct OppIr_Value val;
	enum OppIr_Const_Type loc_type;
	bool global;
};

struct OppIr_Arith {
	int type;
	bool imm;
	struct OppIr_Const val;
};

enum {
	BIT_AND, BIT_OR
};

struct OppIr_Bit {
	int type;
	bool imm, lookback;
	struct OppIr_Const val;
};

struct OppIr_Cast {
	enum OppIr_Const_Type type;
};

struct OppIr_Opcode {
	enum OppIr_Opcode_Type type;

	union {
		struct OppIr_Const constant;
		struct OppIr_Func  func;
		struct OppIr_Var   var;
		struct OppIr_Jmp   jmp;
		struct OppIr_Cmp   cmp;
		struct OppIr_Arith arith;
		struct OppIr_Set   set;
		struct OppIr_Bit   bit;
		struct OppIr_Arg   arg;
		struct OppIr_Cast  cast;
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

struct OppIr_Data {
	size_t allocated, idx;
	unsigned char* data;
};

struct OppIr {
	struct Bytecode code;
	struct Allocation regalloc;
	struct Reg_Stack reg_stack;
	struct Stack local_stack;
	struct OppIr_Instr* instr;
	struct OppIr_Offsets offsets;
	struct OppIr_Data data_seg;
};

#ifdef LINUX64
#	include "ir-linux.h"
#endif

#ifdef MAC64
// #	include "ir-mac.h"
#endif

struct OppIr* init_oppir();
void oppir_free(struct OppIr* ir);
void oppir_setup(OppIO* io);
void oppir_get_opcodes(struct OppIr *ir, struct OppIr_Instr* instr);
void oppir_eval(struct OppIr* ir);
void oppir_eval_opcode(struct OppIr* ir, struct OppIr_Opcode* op);
void oppir_emit_obj(struct OppIr* ir, OppIO* out);
void dump_bytes(struct OppIr* ir, OppIO* io);

// Util
void oppir_check_realloc(struct OppIr* ir, unsigned int bytes);
static enum Regs oppir_push_reg(struct OppIr* ir, enum OppIr_Const_Type type);
static void oppir_push(struct OppIr* ir, enum Regs reg);
static enum Regs oppir_pop_reg(struct OppIr* ir);
static enum Regs oppir_reg_alloc(struct OppIr* ir);
static void oppir_write_const(struct OppIr* ir, struct OppIr_Value* imm);
static int32_t oppir_get_spill(struct OppIr* ir);
static void oppir_set_offsets(struct OppIr* ir);
static void oppir_save_reg(struct OppIr* ir, enum Regs reg_type, struct OppIr_Set* set);
static void oppir_write_reg(struct OppIr* ir, enum Regs reg_type, struct OppIr_Const* imm);

// Opcodes
static void oppir_eval_const(struct OppIr* ir, struct OppIr_Const* imm);
static void oppir_local_param(struct OppIr* ir, unsigned int args);
static void oppir_eval_func(struct OppIr* ir, struct OppIr_Func* fn);
static void oppir_eval_arg(struct OppIr* ir, struct OppIr_Arg* arg);
static void oppir_eval_label(struct OppIr* ir, struct OppIr_Const* loc);
static void oppir_eval_jmp(struct OppIr* ir, struct OppIr_Jmp* jmp);
static void oppir_eval_var(struct OppIr* ir, struct OppIr_Var* var);
static void oppir_eval_cmp(struct OppIr* ir, struct OppIr_Cmp* cmp);
static void oppir_eval_arith(struct OppIr* ir, struct OppIr_Arith* arith);
static void oppir_eval_set(struct OppIr* ir, struct OppIr_Set* set);
static void oppir_eval_cast(struct OppIr* ir, struct OppIr_Cast* cast);
static void oppir_eval_bitwise(struct OppIr* ir, struct OppIr_Bit* bit);
static void oppir_eval_end(struct OppIr* ir);
static void oppir_eval_ret(struct OppIr* ir);

#endif /* OPP_IR */