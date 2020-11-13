/** @file compiler.c
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

	opp->info.cur_ns = opp->global_ns;
	opp->info.fn_name = NULL;
	opp->info.stack_offset = 0;

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
}

static void opp_compile_error(struct Opp_Context* opp, 
		struct Opp_Node* node, const char* str, ...)
{
	va_list ap;
	va_start(ap, str);
	
	printf("%s:%u:%u ", opp->parser->lex->io.fname, 
			node->debug.line, node->debug.colum);
	#ifdef UNX
		printf(CL_RED);
	#endif

	if (opp->info.fn_name != NULL)
		printf("compile time error: in function \"%s(...)\"\n\t", opp->info.fn_name);
	else
		printf("compile time error: in declaration\n\t");

	#ifdef UNX
		printf(CL_RESET);
	#endif 
	vfprintf(stdout, str, ap);
	fprintf(stdout, "\n");
	va_end(ap);
	exit(1);
}

static void opp_warning(struct Opp_Context* opp, 
		struct Opp_Node* node, const char* str, ...)
{
	va_list ap;
	va_start(ap, str);
	
	printf("%s:%u:%u ", opp->parser->lex->io.fname, 
			node->debug.line, node->debug.colum);
	#ifdef UNX
		printf(CL_YELLOW);
	#endif

	printf("warning: in function \"%s(...)\"\n\t", opp->info.fn_name);

	#ifdef UNX
		printf(CL_RESET);
	#endif 
	vfprintf(stdout, str, ap);
	fprintf(stdout, "\n");
	va_end(ap);
}

static inline void opp_realloc_instrs(struct Opp_Context* opp)
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
	for (int stmt = 0; stmt < opp->parser->nstmts; stmt++) {
		switch (opp->parser->statments[stmt]->type)
		{
			case STMT_FUNC:
				opp_compile_func(opp, opp->parser->statments[stmt]);
				break;

			case STMT_EXTERN:
				// opp_compile_extern(opp, opp->parser->statments[stmt]);
				break;

			case STMT_IMPORT:
				opp_import_module(opp, opp->parser->statments[stmt]);
				break;
		}
	}
}

static void opp_compile_stmt(struct Opp_Context* opp, struct Opp_Node* stmt)
{
	switch (stmt->type)
	{
		case STMT_LABEL:
		case STMT_GOTO: 
		case STMT_IF:
		case STMT_WHILE:

		case STMT_RET:
			break;

		case STMT_VAR:
			opp_compile_var(opp, stmt);
			break;

		case STMT_BLOCK:
			opp_compile_block(opp, stmt);
			break;

		case EBIN: case ELOGIC: case EUNARY: case ESUB:
		case EELEMENT: case EDEREF:
		case EADDR: case ESIZEOF:
			opp_warning(opp, stmt, "Unused expression");
			break;

		default:
			opp_compile_expr(opp, stmt);
	}
}

static void opp_import_module(struct Opp_Context* opp, struct Opp_Node* module)
{
	
}

static void opp_compile_block(struct Opp_Context* opp, struct Opp_Node* block)
{
	struct Opp_Namespace* temp = opp->info.cur_ns;

	opp->info.cur_ns = init_namespace(opp->info.cur_ns, (void*)alloc);

	for (unsigned int i = 0; i < block->block_stmt.len; i++)
		opp_compile_stmt(opp, block->block_stmt.stmts[i]);

	opp->info.cur_ns = temp;
}

static void opp_compile_expr(struct Opp_Context* opp, struct Opp_Node* expr)
{
	switch (expr->type)
	{
		case EBIN:
			opp_compile_bin(opp, expr);
			break;
	}
}

static void opp_compile_args(struct Opp_Context* opp, struct Opp_Node* args)
{
	struct Opp_Stmt_Func* fn = &args->fn_stmt;
	struct Opp_Bucket* arg = NULL;
	for (unsigned int i = 0; i < fn->args->length; i++) {
		switch (fn->args->list[i]->type)
		{
			case EUNARY:
				arg = env_add_item(opp->info.cur_ns, fn->args->list[i]->unary_expr.val.strval);
				arg->type = TYPE_LOCAL;
				arg->var.offset = opp->info.stack_offset - 8;
				arg->var.type = TYPE_NUM;
				break;

			case EDEREF: {
				unsigned int depth = 0;
				struct Opp_Node* node = fn->args->list[i];

				while (node->type != EUNARY) {
					if (node->type != EDEREF)
						opp_compile_error(opp, args, "Error when evaluating pointer depth in argument #%d",
							(i+1));
					node = node->defer_expr.defer;
					depth++;
				}
				arg = env_add_item(opp->info.cur_ns, node->unary_expr.val.strval);
				arg->type = TYPE_LOCAL;
				arg->var.offset = opp->info.stack_offset - 8;
				arg->var.type = TYPE_PTR;
				arg->var.ptr_depth = depth;
				break;
			}

			default:
				opp_compile_error(opp, args, "Unexpected argument type in parameter #%d",
					(i+1));
		}

		opp->info.stack_offset -= 8;
	}
}

static void opp_compile_func(struct Opp_Context* opp, struct Opp_Node* func)
{
	opp_realloc_instrs(opp);

	struct Opp_Stmt_Func* fn = &func->fn_stmt;
	char* const name = fn->name->unary_expr.val.strval;
	opp->info.fn_name = name;
	opp->info.stack_offset = 0;

	struct Opp_Bucket* fn_node = env_add_item(opp->global_ns, name);

	if (fn->body->type != STMT_BLOCK)
		opp_compile_error(opp, func, "Expected a block statement after function");

	opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_FUNC; 
	opp->ir.opcodes[opp->ir.instr_idx].func.fn_name = name;
	opp->ir.opcodes[opp->ir.instr_idx].func.private = 0;

	allocator_reset();
	opp->info.cur_ns = init_namespace(opp->global_ns, (void*)alloc);

	if (fn->args->length > 6)
		opp_compile_error(opp, func, "Error function parameter amount is above limit (6) provided '%u'",
			fn->args->length);

	opp_compile_args(opp, func);
	fn_node->func.args = fn->args->length;
	opp->ir.opcodes[opp->ir.instr_idx].func.args = fn_node->func.args; 
	opp->ir.instr_idx++;

	for (unsigned int i = 0; i < fn->body->block_stmt.len; i++)
		opp_compile_stmt(opp, fn->body->block_stmt.stmts[i]);

	opp->info.fn_name = NULL;

	opp_realloc_instrs(opp);
	opp->ir.opcodes[opp->ir.instr_idx++].type = OPCODE_END;
}

static void opp_compile_ret(struct Opp_Context* opp, struct Opp_Node* ret)
{

}

static void opp_compile_var(struct Opp_Context* opp, struct Opp_Node* var)
{
	bool global = (opp->info.cur_ns == opp->global_ns) ? 1 : 0;

	for (unsigned int i = 0; i < var->var_stmt.vars->length; i++) {
		if (var->var_stmt.vars->list[i]->type != EASSIGN)
			opp_compile_error(opp, var, "Expected assigment '=' in auto declaration");

		if (var->var_stmt.vars->list[i]->assign_expr.op != TEQ)
			opp_compile_error(opp, var, "Expected '=' in auto declaration");

		opp_compile_expr(opp, var->var_stmt.vars->list[i]->assign_expr.val);

		opp_realloc_instrs(opp);

		opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_VAR;
		opp->ir.opcodes[opp->ir.instr_idx].var.global = global;

		if (global)
			opp->ir.opcodes[opp->ir.instr_idx].var.name = var->var_stmt.vars->list[i]->
				assign_expr.ident->unary_expr.val.strval;
		else {
			// Add array type
			opp->ir.opcodes[opp->ir.instr_idx].var.offset = opp->info.stack_offset - 8;
			opp->info.stack_offset -= 8;
		}
		opp->ir.instr_idx++;
	}
}

static void opp_compile_label(struct Opp_Context* opp, struct Opp_Node* lab)
{

}

static void opp_compile_goto(struct Opp_Context* opp, struct Opp_Node* lab)
{

}

static void opp_compile_if(struct Opp_Context* opp, struct Opp_Node* ifstmt)
{

}

static void opp_compile_extern(struct Opp_Context* opp, struct Opp_Node* extrn)
{

}

static void opp_compile_while(struct Opp_Context* opp, struct Opp_Node* loop)
{

}

static void opp_compile_unary(struct Opp_Context* opp, struct Opp_Node* unary)
{
	opp_realloc_instrs(opp);

	opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_CONST;

	switch (unary->unary_expr.type)
	{
		case TINTEGER: 
			opp->ir.opcodes[opp->ir.instr_idx].constant.type = IMM_I64;
			opp->ir.opcodes[opp->ir.instr_idx].constant.imm_i64 = unary->unary_expr.val.i64val;
			break;

		case TSTR:
			opp->ir.opcodes[opp->ir.instr_idx].constant.type = IMM_STR;
			opp->ir.opcodes[opp->ir.instr_idx].constant.imm_sym = unary->unary_expr.val.strval;
			break;

		case TIDENT:
			opp_compile_literal(opp, unary);
			break;
	}

	opp->ir.instr_idx++;
}

static void opp_compile_literal(struct Opp_Context* opp, struct Opp_Node* unary)
{
	opp->ir.opcodes[opp->ir.instr_idx].constant.type = IMM_LOC;

	struct Opp_Bucket* sym = env_get_item(opp->info.cur_ns, unary->unary_expr.val.strval);

	if (sym == NULL)
		opp_compile_error(opp, unary, "Use of undefined value '%s'", 
			unary->unary_expr.val.strval);

	if (opp->info.cur_ns == opp->global_ns) {
		opp->ir.opcodes[opp->ir.instr_idx].constant.global = 1;
		printf("ADD GLOBAL\n");
	}
	else {
		opp->ir.opcodes[opp->ir.instr_idx].constant.global = 0;
		opp->ir.opcodes[opp->ir.instr_idx].constant.imm_i32 = sym->var.offset;
	}
}


static struct Opp_Node* opp_compile_time_eval(struct Opp_Context* opp, struct Opp_Node* expr)
{
	switch (expr->type)
	{
		case EUNARY: {
			if (expr->unary_expr.type == TSTR)
				opp_compile_error(opp, expr,
					"Unexpected '%s' type in expression", expr->unary_expr.val.strval);
			return expr;
		}

		case EBIN: {
			struct Opp_Node* lhs = opp_compile_time_eval(opp, expr->bin_expr.left);
			struct Opp_Node* rhs = opp_compile_time_eval(opp, expr->bin_expr.right);
	
			if (lhs == NULL || rhs == NULL) {
				return NULL;
			}

			if (lhs->unary_expr.type == TINTEGER && rhs->unary_expr.type == TINTEGER) {
				const int op = expr->bin_expr.tok;
				struct Opp_Debug temp = {.line = expr->debug.line, .colum = expr->debug.colum};
				memset(expr, 0, sizeof(struct Opp_Node));

				expr->type = EUNARY;
				expr->unary_expr.type = TINTEGER;
				expr->debug = temp;

				switch (op)
				{
					case TADD:
						expr->unary_expr.val.i64val = lhs->unary_expr.val.i64val + rhs->unary_expr.val.i64val;
						break;

					case TMIN:
						expr->unary_expr.val.i64val = lhs->unary_expr.val.i64val - rhs->unary_expr.val.i64val;
						break;

					case TMUL:
						expr->unary_expr.val.i64val = lhs->unary_expr.val.i64val * rhs->unary_expr.val.i64val;
						break;

					case TDIV:
						expr->unary_expr.val.i64val = lhs->unary_expr.val.i64val / rhs->unary_expr.val.i64val;
						break;

					case TMOD:
						expr->unary_expr.val.i64val = lhs->unary_expr.val.i64val % rhs->unary_expr.val.i64val;
						break;
				}

				return expr;
			}

			break;
		}

		case ELOGIC: {
			break;
		}
	}
	return NULL;
}

static void opp_compile_bin(struct Opp_Context* opp, struct Opp_Node* bin)
{
	opp_compile_time_eval(opp, bin);

	if (bin->type == EUNARY) {
		opp_compile_unary(opp, bin);
		return;
	}

}

static void opp_compile_assign(struct Opp_Context* opp, struct Opp_Node* assign)
{

}

static void opp_compile_call(struct Opp_Context* opp, struct Opp_Node* call)
{

}

static void opp_compile_logic(struct Opp_Context* opp, struct Opp_Node* logic)
{

}

static void opp_compile_addr(struct Opp_Context* opp, struct Opp_Node* expr)
{

}

static void opp_compile_deref(struct Opp_Context* opp, struct Opp_Node* expr)
{

}

static void opp_compile_ptr_assign(struct Opp_Context* opp, struct Opp_Node* expr)
{

}