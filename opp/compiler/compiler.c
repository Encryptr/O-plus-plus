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

struct Opp_Context* opp_init_compile(struct Opp_Parser* parser, 
									struct Opp_Options* opts)
{
	struct Opp_Context* opp = (struct Opp_Context*)
		malloc(sizeof(struct Opp_Context));

	if (opp == NULL)
		goto err;

	opp->parser = parser;
	opp->opts = opts;
	opp->global_ns = init_namespace(NULL, malloc);
	opp->oppir = NULL;

	// Info
	// check whats needed

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

	return opp;

	err:
		INTERNAL_ERROR("Malloc Fail");
		return NULL;
}

void opp_free_compiler(struct Opp_Context* opp)
{
	free(opp->global_ns); // free the ns last
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

static void opp_compile_func(struct Opp_Context* opp, struct Opp_Node* func)
{

}

// static void opp_compile_stmt(struct Opp_Context* opp, struct Opp_Node* stmt)
// {
// 	switch (stmt->type)
// 	{
// 		case STMT_LABEL:
// 			opp_compile_label(opp, stmt);
// 			break;

// 		case STMT_GOTO:
// 			opp_compile_goto(opp, stmt);
// 			break;

// 		case STMT_RET:
// 			opp_compile_ret(opp, stmt);
// 			break;

// 		case STMT_IF:
// 			opp_compile_if(opp, stmt);
// 			break;

// 		case STMT_WHILE:
// 			opp_compile_while(opp, stmt);
// 			break;

// 		case STMT_FOR:
// 		case STMT_SWITCH:
// 			break;

// 		case STMT_VAR:
// 			opp_compile_var(opp, stmt);
// 			break;

// 		case STMT_BLOCK:
// 			opp_compile_block(opp, stmt);
// 			break;

// 		case STMT_BREAK:
// 			if (opp->info.state != STATE_LOOP)
// 				opp_compile_error(opp, stmt,
// 					"Unexpected 'break' statement outide a loop");
// 			break;

// 		case STMT_CASE:
// 			if (opp->info.state != STATE_SWITCH)
// 				opp_compile_error(opp, stmt,
// 					"Unexpected 'case' statement outside 'switch'");
// 			break;
		
// 		case EBIN: case ELOGIC: case EUNARY: case ESUB:
// 		case EELEMENT: case EDEREF:
// 		case EADDR: case ESIZEOF:
// 			opp_warning(opp, stmt, "Unused expression");
// 			break;

// 		default: 
// 			opp_compile_expr(opp, stmt);
// 	}
// }