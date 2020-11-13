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

struct Opp_Options {
	bool dump_toks;
	bool run_output;
	bool link_c;
	bool nostd;
};

struct Opp_Info {
	struct Opp_Namespace* cur_ns;
	int32_t stack_offset;
	char* fn_name;
};

#define DEFAULT_OPCODE_SIZE 64

struct Opp_Context {
	struct Opp_Parser* parser;
	struct Opp_Options* opts;
	struct Opp_Info info;
	struct OppIr_Instr ir;
	struct OppIr* oppir;
	struct Opp_Namespace* global_ns;
};

struct Opp_Context* opp_init_compile(struct Opp_Parser* parser, 
									struct Opp_Options* opts);
void opp_compile(struct Opp_Context* opp);
static void opp_compile_error(struct Opp_Context* opp, 
		struct Opp_Node* node, const char* str, ...);
static void opp_warning(struct Opp_Context* opp, 
		struct Opp_Node* node, const char* str, ...);

static void opp_compile_stmt(struct Opp_Context* opp, struct Opp_Node* stmt);
static void opp_compile_expr(struct Opp_Context* opp, struct Opp_Node* expr);
static struct Opp_Node* opp_compile_time_eval
	(struct Opp_Context* opp, struct Opp_Node* expr);

// Expr
static void opp_compile_unary(struct Opp_Context* opp, struct Opp_Node* unary);
static void opp_compile_bin(struct Opp_Context* opp, struct Opp_Node* bin);
static void opp_compile_assign(struct Opp_Context* opp, struct Opp_Node* assign);
static void opp_compile_call(struct Opp_Context* opp, struct Opp_Node* call);
static void opp_compile_logic(struct Opp_Context* opp, struct Opp_Node* logic);
static void opp_compile_literal(struct Opp_Context* opp, struct Opp_Node* unary);
static void opp_compile_addr(struct Opp_Context* opp, struct Opp_Node* expr);
static void opp_compile_deref(struct Opp_Context* opp, struct Opp_Node* expr);
static void opp_compile_ptr_assign(struct Opp_Context* opp, struct Opp_Node* expr);
static void opp_import_module(struct Opp_Context* opp, struct Opp_Node* module);

// Stmts
static void opp_compile_func(struct Opp_Context* opp, struct Opp_Node* func);
static void opp_compile_ret(struct Opp_Context* opp, struct Opp_Node* ret);
static void opp_compile_block(struct Opp_Context* opp, struct Opp_Node* block);
static void opp_compile_var(struct Opp_Context* opp, struct Opp_Node* var);
static void opp_compile_label(struct Opp_Context* opp, struct Opp_Node* lab);
static void opp_compile_goto(struct Opp_Context* opp, struct Opp_Node* lab);
static void opp_compile_if(struct Opp_Context* opp, struct Opp_Node* ifstmt);
static void opp_compile_extern(struct Opp_Context* opp, struct Opp_Node* extrn);
static void opp_compile_while(struct Opp_Context* opp, struct Opp_Node* loop);

#endif /* OPP_COMPILER */