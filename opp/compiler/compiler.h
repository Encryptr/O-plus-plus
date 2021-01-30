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
	unsigned int depth, size;
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

#endif /* OPP_COMPILER */