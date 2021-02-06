/*
 * @file compiler.h
 * 
 * @brief Opp compiler module 
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

#ifndef OPP_COMPILER
#define OPP_COMPILER

#include <stdio.h>
#include <stdbool.h>
#include "../parser/parser.h"
#include "../ir/ir.h"
#include "../env/env.h"
#include "../ast/ast.h"

#define UINT_MAX 4294967295

struct Opp_Options {
	bool dump_toks,
		run_output,
		link_c,
		nostd,
		warning,
		debug,
		wall,
		test,
		live;
};

enum Opp_Cond_Type {
	LOGIC_COND,
	IFWHILE_COND,
	SWITCH_COND
};

struct Opp_Cond {
	enum Opp_Cond_Type cond_type;
	bool in_logic;
	unsigned int locs[2], endloc;
	int curr_logic;
	bool use_op;
	unsigned int jloc, hs;
};

struct Opp_Info {
	Opp_Obj ret_type;
	int32_t stack_offset;
	unsigned int label_loc;

	// Dot expr info
	struct {
		bool indot;
		struct Opp_Bucket* sym;
		int32_t offset;
		bool inptr;
	} dot;

	bool need_addr;
	bool in_deref;
};

#define DEFAULT_OPCODE_SIZE 64
#define OPCODE_TYPE(op_type) \
	opp->ir.opcodes[opp->ir.instr_idx].type = op_type
#define OPCODE_START() \
	opp_realloc_instrs(opp)
#define OPCODE_END() \
	opp->ir.instr_idx++

#define NEED_ADDR() \
	opp->info.need_addr = true
#define REVERT() \
	opp->info.need_addr = false

struct Opp_Context {
	struct Opp_Parser* parser;
	struct Opp_Options* opts;
	struct Opp_Info info;
	struct OppIr_Instr ir;
	struct OppIr* oppir;
	struct Opp_Cond cond_state;
};

struct Opp_Context* opp_init_compile(struct Opp_Parser* parser, 
									struct Opp_Options* opts);
void opp_free_compiler(struct Opp_Context* opp);
void opp_compile(struct Opp_Context* opp);
void opp_realloc_instrs(struct Opp_Context* opp);
void opp_compile_stmt(struct Opp_Context* opp, struct Opp_Node* node);
Opp_Obj opp_compile_expr(struct Opp_Context* opp, struct Opp_Node* expr);
unsigned int opp_type_to_size(Opp_Obj* type);
enum OppIr_Const_Type opp_type_to_ir(Opp_Obj* type);
void opp_compile_var(struct Opp_Context* opp, struct Opp_Node* node);
void opp_type_cast(struct Opp_Context* opp, Opp_Obj* lhs, Opp_Obj* rhs);
Opp_Obj opp_compile_call(struct Opp_Context* opp, struct Opp_Node* call, bool stmt);
void opp_create_label(struct Opp_Context* opp, int32_t jump_loc);

#endif /* OPP_COMPILER */