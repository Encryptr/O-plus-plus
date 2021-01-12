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

struct Opp_Info {
	int32_t stack_offset;
	unsigned int label_loc, sym_loc;
	bool deref_assign;
};

struct Opp_Elem {
	enum Opp_Bucket_Type type;
	struct Opp_Type_Decl decl;
	unsigned int sym_tab_loc;
};

struct Opp_Env {
	struct Opp_Elem* global_table;
	struct Opp_Elem* local_table;
	size_t allocated;
};

#define DEFAULT_OPCODE_SIZE 64

struct Opp_Context {
	struct Opp_Parser* parser;
	struct Opp_Options* opts;
	struct Opp_Info info;
	struct OppIr_Instr ir;
	struct OppIr* oppir;
	struct Opp_Cond cond_state;
	struct Opp_Env comp_env;
};

struct Opp_Context* opp_init_compile(struct Opp_Parser* parser, 
									struct Opp_Options* opts);
void opp_free_compiler(struct Opp_Context* opp);
void opp_compile(struct Opp_Context* opp);

static void opp_compile_stmt(struct Opp_Context* opp, struct Opp_Node* stmt);
static struct Opp_Type opp_compile_expr(struct Opp_Context* opp, struct Opp_Node* expr);
static void opp_set_offsets(struct Opp_Context* opp);
static void opp_generate_ret(struct Opp_Context* opp);

// Expr
static struct Opp_Type opp_compile_unary(struct Opp_Context* opp, struct Opp_Node* unary);
static struct Opp_Type opp_compile_bin(struct Opp_Context* opp, struct Opp_Node* bin);
static struct Opp_Type opp_compile_sub(struct Opp_Context* opp, struct Opp_Node* sub);
static struct Opp_Type opp_compile_assign(struct Opp_Context* opp, struct Opp_Node* assign);
static struct Opp_Type opp_compile_call(struct Opp_Context* opp, struct Opp_Node* call);
static struct Opp_Type opp_compile_logic(struct Opp_Context* opp, struct Opp_Node* logic);
static struct Opp_Type opp_compile_logic_assign(struct Opp_Context* opp, struct Opp_Node* logic);
static void opp_compile_logic_end(struct Opp_Context* opp, struct Opp_Node* logic);
static struct Opp_Type opp_compile_literal(struct Opp_Context* opp, struct Opp_Node* unary);
static struct Opp_Type opp_compile_addr(struct Opp_Context* opp, struct Opp_Node* expr);
static struct Opp_Type opp_compile_deref(struct Opp_Context* opp, struct Opp_Node* expr);
static struct Opp_Type opp_compile_ptr_assign(struct Opp_Context* opp, struct Opp_Node* expr);
static void opp_import_module(struct Opp_Context* opp, struct Opp_Node* module);

// Stmts
static void opp_compile_func(struct Opp_Context* opp, struct Opp_Node* func);
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

// Debug
void opp_debug_node(struct Opp_Node* base);

#endif /* OPP_COMPILER */