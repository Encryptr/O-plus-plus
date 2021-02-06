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

static void opp_compile_func(struct Opp_Context* opp, struct Opp_Node* node);
static void opp_compile_extern(struct Opp_Context* opp, struct Opp_Node* node);

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
	// opp->env.global_ns = parser->scope;
	// opp->env.curr_ns = opp->env.global_ns;

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

void opp_realloc_instrs(struct Opp_Context* opp)
{
	if (opp->ir.instr_idx >= opp->ir.allocated) {
		opp->ir.allocated += 64;
		opp->ir.opcodes = (struct OppIr_Opcode*)realloc(opp->ir.opcodes, 
			(opp->ir.allocated) * sizeof(struct OppIr_Opcode));

		if (opp->ir.opcodes == NULL)
			INTERNAL_ERROR("Realloc Fail");
	}
}

enum OppIr_Const_Type opp_type_to_ir(Opp_Obj* type)
{
	int t = 0;
	if (type->depth > 0 || 
		type->decl->t_type == TYPE_STRUCT ||
		type->size > 0)
		return IMM_U64;

	switch (type->decl->t_type)
	{
		case TYPE_I8:
			t = IMM_I8;
			break;

		case TYPE_I16:    
			t = IMM_I16;
			break;

		case TYPE_I32:    
			t = IMM_I32;
			break;

		case TYPE_I64:    
			t = IMM_I64;
			break;

		case TYPE_FLOAT:  
			t = IMM_F32;
			break;

		case TYPE_DOUBLE: 
			t = IMM_F64;
			break;
			
		default: break;
	}

	if (type->unsign)
		t--;

	return t;
}

unsigned int opp_type_to_size(Opp_Obj* type)
{
	if (type->depth > 0 && type->size == 0)
		return 8;

	if (type->size > 0)
		return type->size;

	unsigned int size = 0;
	switch (type->decl->t_type)
	{
		case TYPE_I8:
			size = 1;
			break;
		
		case TYPE_I16:
			size = 2;
			break;

		case TYPE_FLOAT:
		case TYPE_I32: 
			size = 4;
			break;

		case TYPE_DOUBLE:
		case TYPE_I64:
			size = 8;
			break;

		case TYPE_STRUCT:
			size = type->decl->size;
			break;

		default: break;
	}
	return size;
}

void opp_type_cast(struct Opp_Context* opp, Opp_Obj* lhs, Opp_Obj* rhs)
{
	/* Cast rules
		1) Always cast based on lhs
		Ex: 
			double a;
			int b;
			b = a; # Cast a to int

		2) Only create a cast if
			- Lhs and Rhs are different floating points
			- Lhs and Rhs are different types 
				not including integer types.
	*/
	enum OppIr_Const_Type lt = opp_type_to_ir(lhs);
	enum OppIr_Const_Type rt = opp_type_to_ir(rhs);

	if (
		((lt == IMM_F64 || lt == IMM_F32) && (rt != IMM_F32 && rt != IMM_F64))
		||
		((lt != IMM_F64 && lt != IMM_F32) && (rt == IMM_F64 || rt == IMM_F32))
		||
		((lt == IMM_F64 && rt == IMM_F32) || (lt == IMM_F32 && rt == IMM_F64))
		) 
	{
		printf("CASTING\n");
		OPCODE_START();
			OPCODE_TYPE(OPCODE_CAST);
			opp->ir.opcodes[opp->ir.instr_idx].cast.type = lt;
		OPCODE_END();
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

			case STMT_EXTERN:
				opp_compile_extern(opp, opp->parser->statments[stmt]);
				break;

			case STMT_VAR:
				opp_compile_var(opp, opp->parser->statments[stmt]);
				break;

			default: break;
		}
	}
}

static void opp_compile_args(struct Opp_Context* opp, struct Opp_Node* node)
{
	unsigned int Fidx = 0, Iidx = 0;

	for (unsigned int i = 0; i < node->fn_stmt.len; i++)
	{
		OPCODE_START();
			OPCODE_TYPE(OPCODE_ARG);
			opp->ir.opcodes[opp->ir.instr_idx].arg.type = 
				opp_type_to_ir(&node->fn_stmt.args[i].var_stmt.type);

			switch (opp->ir.opcodes[opp->ir.instr_idx].arg.type)
			{
				case IMM_U8...IMM_I64: 
					opp->ir.opcodes[opp->ir.instr_idx].arg.idx = Iidx;
					Iidx++;
					break;

				case IMM_F32...IMM_F64:
					opp->ir.opcodes[opp->ir.instr_idx].arg.idx = Fidx;
					Fidx++;
					break;

				default: break;
			}

		OPCODE_END();

		opp->info.stack_offset -= 8;
		node->fn_stmt.args[i].var_stmt.bidx->offset = opp->info.stack_offset; 
	}
}

static void opp_compile_func(struct Opp_Context* opp, struct Opp_Node* node)
{
	struct Opp_Stmt_Func* func = &node->fn_stmt;

	// Resets
	opp->info.stack_offset = 0;
	opp->info.label_loc    = 1;
	opp->info.ret_type = func->type;

	OPCODE_START();
		OPCODE_TYPE(OPCODE_FUNC);
		opp->ir.opcodes[opp->ir.instr_idx].func.fn_name = func->bidx->key;
		opp->ir.opcodes[opp->ir.instr_idx].func.ext = 0;
	OPCODE_END();

	if (func->body == NULL)
		return;

	opp_compile_args(opp, node);

	opp_compile_stmt(opp, func->body);

	OPCODE_START();
		OPCODE_TYPE(OPCODE_END);
	OPCODE_END();
}

static void opp_compile_extern(struct Opp_Context* opp, struct Opp_Node* node)
{
	assert(node->extrn_stmt.stmt->type == STMT_FUNC);

	OPCODE_START();
		if (node->extrn_stmt.stmt->type == STMT_FUNC) {
			OPCODE_TYPE(OPCODE_FUNC);
			opp->ir.opcodes[opp->ir.instr_idx].func.fn_name = 
				node->extrn_stmt.stmt->fn_stmt.name->unary_expr.val.strval;
			opp->ir.opcodes[opp->ir.instr_idx].func.ext = 1;
		}
	OPCODE_END();

}
