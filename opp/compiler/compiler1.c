/*
 * @file compiler.c
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

#include "compiler.h"

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

struct Opp_Context* opp_init_compile(struct Opp_Parser* parser, 
									struct Opp_Options* opts)
{
	struct Opp_Context* opp = (struct Opp_Context*)
		malloc(sizeof(struct Opp_Context));

	if (opp == NULL)
		goto err;

	opp->parser = parser;
	opp->opts = opts;
	opp->oppir = NULL;

	// Info
	memset(&opp->info, 0, sizeof(struct Opp_Info));

	// Cond
	memset(&opp->cond_state, 0, sizeof(struct Opp_Cond));

	// Ir
	opp->ir.instr_idx = 0;
	opp->ir.allocated = DEFAULT_OPCODE_SIZE;
	opp->ir.opcodes = (struct OppIr_Opcode*)
		malloc(sizeof(struct OppIr_Opcode)*DEFAULT_OPCODE_SIZE);

	if (opp->ir.opcodes == NULL)
		goto err;

	memset(opp->ir.opcodes, 0, sizeof(struct OppIr_Opcode)*DEFAULT_OPCODE_SIZE);

	// Tables
	opp->env.global_ns = parser->scope;
	opp->env.curr_ns = opp->env.global_ns;

	return opp;

err:
	INTERNAL_ERROR("Malloc Fail");
	return NULL;
}

void opp_free_compiler(struct Opp_Context* opp)
{
	free(opp->ir.opcodes);
	free(opp);
}

static void opp_realloc_instrs(struct Opp_Context* opp)
{
	if (opp->ir.instr_idx >= opp->ir.allocated) {
		opp->ir.allocated += 64;
		opp->ir.opcodes = (struct OppIr_Opcode*)realloc(opp->ir.opcodes, 
			(opp->ir.allocated) * sizeof(struct OppIr_Opcode));

		if (opp->ir.opcodes == NULL)
			INTERNAL_ERROR("Realloc Fail");
	}
}

void opp_compile(struct Opp_Context* opp)
{
	for (unsigned int stmt = 0; stmt < opp->parser->nstmts; stmt++) {
		switch (opp->parser->statments[stmt]->type)
		{
			case STMT_FUNC:
				opp_compile_func(opp, opp->parser->statments[stmt]);
				break;

			// case STMT_STRUCT:
			// 	opp_compile_struct(opp, opp->parser->statments[stmt]);
			// 	break;

			// case STMT_EXTERN:
			// 	opp_compile_extern(opp, opp->parser->statments[stmt]);
			// 	break;

			// case STMT_IMPORT:
			// 	opp_import_module(opp, opp->parser->statments[stmt]);
			// 	break;

			// case STMT_VAR:
			// 	opp_compile_var(opp, opp->parser->statments[stmt]);
			// 	break;

			default: break;
		}
	}
}

static void opp_compile_stmt(struct Opp_Context* opp, struct Opp_Node* stmt)
{

}

static Opp_Type opp_compile_expr(struct Opp_Context* opp, struct Opp_Node* expr)
{
	
}