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
		wall;
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

typedef struct Opp_Type {
	char type;
	char comp_type;
	unsigned int depth;
} Opp_Type;


struct Opp_Info {
	Opp_Type ret_type;
	int32_t stack_offset;
	unsigned int label_loc;

	// Dot expr info
	struct {
		bool indot;
		struct Opp_Stmt_Struct* s_ns;
	} dot;

	bool in_assign, in_addr;
};

struct Opp_Env {
	struct Opp_Namespace* global_ns;
	struct Opp_Namespace* curr_ns;
};

#define DEFAULT_OPCODE_SIZE 64

struct Opp_Context {
	struct Opp_Parser* parser;
	struct Opp_Options* opts;
	struct Opp_Info info;
	struct OppIr_Instr ir;
	struct OppIr* oppir;
	struct Opp_Cond cond_state;
	struct Opp_Env env;
};

struct Opp_Context* opp_init_compile(struct Opp_Parser* parser, 
									struct Opp_Options* opts);
void opp_free_compiler(struct Opp_Context* opp);
void opp_compile(struct Opp_Context* opp);

static void opp_compile_stmt(struct Opp_Context* opp, struct Opp_Node* stmt);
static Opp_Type opp_compile_expr(struct Opp_Context* opp, struct Opp_Node* expr);
static void opp_set_offsets(struct Opp_Context* opp);
static Opp_Type opp_type_to_ir(struct Opp_Type_Decl* type);
static void opp_comp_type(Opp_Type* t);
static unsigned int opp_type_to_size(struct Opp_Type_Decl* type);
static void opp_try_cast(struct Opp_Context* opp, Opp_Type lhs, Opp_Type rhs);

// Expr
static Opp_Type opp_compile_unary(struct Opp_Context* opp, struct Opp_Node* unary);
static Opp_Type opp_compile_bin(struct Opp_Context* opp, struct Opp_Node* bin);
static Opp_Type opp_compile_dot(struct Opp_Context* opp, struct Opp_Node* dot);
static Opp_Type opp_compile_sub(struct Opp_Context* opp, struct Opp_Node* sub);
static Opp_Type opp_compile_assign(struct Opp_Context* opp, struct Opp_Node* assign);
static Opp_Type opp_compile_call(struct Opp_Context* opp, struct Opp_Node* call);
static Opp_Type opp_compile_logic(struct Opp_Context* opp, struct Opp_Node* logic);
static Opp_Type opp_compile_logic_assign(struct Opp_Context* opp, struct Opp_Node* logic);
static void opp_compile_logic_end(struct Opp_Context* opp, struct Opp_Node* logic);
static Opp_Type opp_compile_literal(struct Opp_Context* opp, struct Opp_Node* unary);
static Opp_Type opp_compile_addr(struct Opp_Context* opp, struct Opp_Node* expr);
static Opp_Type opp_compile_deref(struct Opp_Context* opp, struct Opp_Node* expr);
static Opp_Type opp_compile_ptr_assign(struct Opp_Context* opp, struct Opp_Node* expr);

// Stmts
static void opp_import_module(struct Opp_Context* opp, struct Opp_Node* module);
static void opp_compile_func(struct Opp_Context* opp, struct Opp_Node* func);
static void opp_compile_struct(struct Opp_Context* opp, struct Opp_Node* node);
static void opp_compile_ret(struct Opp_Context* opp, struct Opp_Node* ret);
static void opp_compile_block(struct Opp_Context* opp, struct Opp_Node* block);
static void opp_compile_var(struct Opp_Context* opp, struct Opp_Node* var);
static void opp_compile_bitfield(struct Opp_Context* opp, struct Opp_Node* bit);
static void opp_compile_label(struct Opp_Context* opp, struct Opp_Node* lab);
static void opp_compile_goto(struct Opp_Context* opp, struct Opp_Node* lab);
static void opp_compile_if(struct Opp_Context* opp, struct Opp_Node* ifstmt);
static void opp_compile_extern(struct Opp_Context* opp, struct Opp_Node* extrn);
static void opp_compile_while(struct Opp_Context* opp, struct Opp_Node* loop);
static void opp_compile_for(struct Opp_Context* opp, struct Opp_Node* loop);
static void opp_compile_switch(struct Opp_Context* opp, struct Opp_Node* cond);

#endif /* OPP_COMPILER */