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

	// Info
	opp->info.state = STATE_DEFAULT;
	opp->info.cur_ns = opp->global_ns;
	opp->info.fn_name = NULL;
	opp->info.stack_offset = 0;
	opp->info.label_loc = 1;
	opp->info.goto_list = (struct Opp_Link*)
		malloc(sizeof(struct Opp_Link)*DEFAULT_OFFSET_TABLE);
	if (opp->info.goto_list == NULL)
		goto err;
	opp->info.goto_idx = 0;

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
	free(opp->info.goto_list);
	free(opp->ir.opcodes);
	free(opp);
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
	printf("\t" CL_GREEN);
	opp_debug_node(node);
	printf(CL_RESET "\n");
	exit(1);
}

static void opp_warning(struct Opp_Context* opp, 
		struct Opp_Node* node, const char* str, ...)
{
	if (!opp->opts->warning)
		return;

	if (opp->opts->wall) {
		opp_compile_error(opp, node, str);
		return;
	}

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
	printf("\t" CL_GREEN);
	opp_debug_node(node);
	printf(CL_RESET "\n");
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

static void opp_check_label(struct Opp_Context* opp, unsigned int idx)
{
	// Possibly remove since IR SHOULD HANDLE LABEL relocation
	if ((opp->info.label_loc+idx) >= DEFAULT_OFFSET_TABLE) {
		printf("NEED MORE LABEL LOC's\n");
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

			case STMT_IMPORT:
				opp_import_module(opp, opp->parser->statments[stmt]);
				break;

			case STMT_VAR:
				opp_compile_var(opp, opp->parser->statments[stmt]);
				break;

			default: break;
		}
	}
}

static void opp_compile_stmt(struct Opp_Context* opp, struct Opp_Node* stmt)
{
	switch (stmt->type)
	{
		case STMT_LABEL:
			opp_compile_label(opp, stmt);
			break;

		case STMT_GOTO:
			opp_compile_goto(opp, stmt);
			break;

		case STMT_RET:
			opp_compile_ret(opp, stmt);
			break;

		case STMT_IF:
			opp_compile_if(opp, stmt);
			break;

		case STMT_WHILE:
		case STMT_FOR:
		case STMT_SWITCH:
			break;

		case STMT_VAR:
			opp_compile_var(opp, stmt);
			break;

		case STMT_BLOCK:
			opp_compile_block(opp, stmt);
			break;

		case STMT_BREAK:
			if (opp->info.state != STATE_LOOP)
				opp_compile_error(opp, stmt,
					"Unexpected 'break' statement outide a loop");
			break;

		case STMT_CASE:
			if (opp->info.state != STATE_SWITCH)
				opp_compile_error(opp, stmt,
					"Unexpected 'case' statement outside 'switch'");
			break;
		
		// case EBIN: case ELOGIC: case EUNARY: case ESUB:
		// case EELEMENT: case EDEREF:
		// case EADDR: case ESIZEOF:
		// 	opp_warning(opp, stmt, "Unused expression");
		// 	break;

		default: 
			opp_compile_expr(opp, stmt);
	}
}

static void opp_import_module(struct Opp_Context* opp, struct Opp_Node* module)
{
	if (module->import_stmt.ident->type != EUNARY)
		opp_compile_error(opp, module, "Expected a string directory after 'import'");

	if (module->import_stmt.ident->unary_expr.type != TSTR)
		opp_compile_error(opp, module, "Expected a string directory after 'import'");

	opp_add_module(module->import_stmt.ident->unary_expr.val.strval, opp);
}

static void opp_compile_block(struct Opp_Context* opp, struct Opp_Node* block)
{
	struct Opp_Namespace* temp = opp->info.cur_ns;

	opp->info.cur_ns = init_namespace(opp->info.cur_ns, (void*)alloc);

	for (unsigned int i = 0; i < block->block_stmt.len; i++)
		opp_compile_stmt(opp, block->block_stmt.stmts[i]);

	opp->info.cur_ns = temp;
}

static struct Opp_Type opp_compile_expr(struct Opp_Context* opp, struct Opp_Node* expr)
{
	struct Opp_Type type;
	switch (expr->type)
	{
		case ECALL:
			type = opp_compile_call(opp, expr);
			break;

		case EASSIGN:
			type = opp_compile_assign(opp, expr);
			break;

		case EBIN:
			type = opp_compile_bin(opp, expr);
			break;

		case EUNARY:
			type = opp_compile_unary(opp, expr);
			break;

		case EDEREF:
			type = opp_compile_deref(opp, expr);
			break;

		case EADDR:
			type = opp_compile_addr(opp, expr);
			break;

		case ELOGIC:
			type = opp_compile_logic(opp, expr);
			break;

		case ESUB:
			type = opp_compile_sub(opp, expr);
			break;

		default: break;
	}

	return type;
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
	struct Opp_Stmt_Func* fn = &func->fn_stmt;
	char* const name = fn->name->unary_expr.val.strval;

	// ==============================
	// Setup / Resets
	opp->info.fn_name = name;
	opp->info.stack_offset = 0;
	opp->info.goto_idx = 0;
	opp->info.label_loc = 1;
	// ==============================

	struct Opp_Bucket* fn_node = env_add_item(opp->global_ns, name);

	if (fn_node == NULL)
		opp_compile_error(opp, func, "Redefinition of function '%s(...)'", name);

	if (fn->body == NULL)
		return;

	if (fn->body->type != STMT_BLOCK)
		opp_compile_error(opp, func, "Expected a block statement after function");

	opp_realloc_instrs(opp);
	opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_FUNC; 
	opp->ir.opcodes[opp->ir.instr_idx].func.fn_name = name;
	opp->ir.opcodes[opp->ir.instr_idx].func.private = 0;

	allocator_reset();
	opp->info.cur_ns = init_namespace(opp->global_ns, (void*)alloc);

	if (fn->args->length > 6)
		opp_compile_error(opp, func, "Error function parameter amount is above limit (6), got '%u'",
			fn->args->length);

	opp_compile_args(opp, func);
	fn_node->func.args = fn->args->length;
	opp->ir.opcodes[opp->ir.instr_idx].func.args = fn_node->func.args; 
	opp->ir.instr_idx++;

	for (unsigned int i = 0; i < fn->body->block_stmt.len; i++)
		opp_compile_stmt(opp, fn->body->block_stmt.stmts[i]);

	// Generate function end label
	opp_generate_ret(opp);

	// Function End
	opp_set_offsets(opp);
	opp->info.fn_name = NULL;
	opp->info.cur_ns = opp->global_ns;
	opp_realloc_instrs(opp);
	opp->ir.opcodes[opp->ir.instr_idx++].type = OPCODE_END;
}

static void opp_generate_ret(struct Opp_Context* opp)
{
	struct Opp_Bucket* label = env_add_item(opp->info.cur_ns, FUNC_END_LABEL);
	label->type = TYPE_LABEL;
	label->var.offset = 0;

	opp_realloc_instrs(opp);
	opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_LABEL;
	opp->ir.opcodes[opp->ir.instr_idx].constant.imm_u32 = 0;
	opp->ir.instr_idx++;
}

static void opp_set_offsets(struct Opp_Context* opp)
{
	for (unsigned int i = 0; i < opp->info.goto_idx; i++) {
		struct Opp_Bucket* label = env_get_item(opp->info.cur_ns, opp->info.goto_list[i].label);

		if (label == NULL)
			opp_compile_error(opp, opp->info.goto_list[i].goto_node, 
				"Label '%s' never defined", opp->info.goto_list[i].label);

		opp->info.goto_list[i].op->jmp.loc = label->var.offset;
	}

	opp->info.goto_idx = 0;
}

static void opp_compile_ret(struct Opp_Context* opp, struct Opp_Node* ret)
{
	if (ret->ret_stmt.value) {
		opp_compile_expr(opp, ret->ret_stmt.value);
		opp_realloc_instrs(opp);
		opp->ir.opcodes[opp->ir.instr_idx++].type = OPCODE_RET;
	}

	opp_realloc_instrs(opp);
	opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_JMP;
	opp->ir.opcodes[opp->ir.instr_idx].jmp.type = PURE_JMP;

	if (opp->info.goto_idx >= DEFAULT_OFFSET_TABLE) 
		opp_compile_error(opp, ret, "Max goto limit met (%d) in 'return' statement",
			DEFAULT_OFFSET_TABLE);

	opp->info.goto_list[opp->info.goto_idx].label = FUNC_END_LABEL;
	opp->info.goto_list[opp->info.goto_idx].op = &opp->ir.opcodes[opp->ir.instr_idx];
	opp->info.goto_list[opp->info.goto_idx].goto_node = ret;
	opp->info.goto_idx++;
	opp->ir.instr_idx++;
}

static void opp_compile_var(struct Opp_Context* opp, struct Opp_Node* var)
{
	bool global = (opp->info.cur_ns == opp->global_ns) ? 1 : 0;
	struct Opp_Bucket* cur_var = NULL;

	for (unsigned int i = 0; i < var->var_stmt.vars->length; i++) {
		bool assign = true;
		unsigned int ptr_depth = 0;
		struct Opp_Node* node = var->var_stmt.vars->list[i];

		if (node->type == EUNARY || node->type == EDEREF)
			assign = false;
		else if (node->type == EASSIGN)
		{
			if (node->assign_expr.op != TEQ)
				opp_compile_error(opp, node, "Expected '=' in auto declaration");
			node = var->var_stmt.vars->list[i]->assign_expr.ident;
		}
		else
			opp_compile_error(opp, node, "Unexpected token in auto declaration");

		while (node->type != EUNARY) {
			if (node->type != EDEREF)
				opp_compile_error(opp, node, "Evaluating pointer depth in 'auto' declaration");
			node = node->defer_expr.defer;
			ptr_depth++;
		}

		if (node->unary_expr.type != TIDENT)
			opp_compile_error(opp, var, "Expected a identifier on left side of '=' in auto decl");

		if (global && assign) {
			// TODO: HANDLE CONST EXPR
			// var->var_stmt.vars->list[i]->assign_expr.val->type != EUNARY
			opp_compile_error(opp, var, "Expected a immidiate value assigment in 'auto' decl in global scope");
		}
		
		struct Opp_Bucket* bucket = env_add_item(opp->info.cur_ns, node->unary_expr.val.strval);
		if (bucket == NULL)
			opp_compile_error(opp, var, "Redefinition of variable '%s'", node->unary_expr.val.strval);

		bucket->type = (global) ? TYPE_GLOBAL : TYPE_LOCAL;
		bucket->var.offset = opp->info.stack_offset - 8;
		
		if (ptr_depth > 0) {
			bucket->var.type = TYPE_PTR;
			bucket->var.ptr_depth = ptr_depth;
		}
		else 
			bucket->var.type = TYPE_NUM;
		
		opp->info.stack_offset -= 8; 

		if (assign) {
			struct Opp_Type res = opp_compile_expr(opp, var->var_stmt.vars->list[i]);

			if (bucket->var.type != res.type)
				opp_warning(opp, var, 
					"Assigning variable '%s' to a different type", node->unary_expr.val.strval);

			if (bucket->var.type == TYPE_PTR && res.type == TYPE_PTR) {
				if (bucket->var.ptr_depth != res.ptr_depth)
					opp_warning(opp, var,
						"Assigning variable '%s' to a pointer depth of '%u'",
							node->unary_expr.val.strval, res.ptr_depth);
			}
		}

		opp_realloc_instrs(opp);
		opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_VAR;
		opp->ir.opcodes[opp->ir.instr_idx].var.global = global;
		opp->ir.opcodes[opp->ir.instr_idx].var.elem = 1;

		if (global) 
			opp->ir.opcodes[opp->ir.instr_idx].var.name = node->unary_expr.val.strval;
	
		opp->ir.instr_idx++;
	}
}

static void opp_compile_label(struct Opp_Context* opp, struct Opp_Node* lab)
{
	char* name = lab->label_stmt.name->unary_expr.val.strval;

	struct Opp_Bucket* label = env_add_item(opp->info.cur_ns, name);

	if (label == NULL)
		opp_compile_error(opp, lab,
			"Redefinition of label '%s:'", name);

	opp_check_label(opp, 1);
	label->type = TYPE_LABEL;
	label->var.offset = opp->info.label_loc;

	opp_realloc_instrs(opp);
	opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_LABEL;
	opp->ir.opcodes[opp->ir.instr_idx].constant.imm_u32 = opp->info.label_loc;
	opp->info.label_loc++;
	opp->ir.instr_idx++;
}

static void opp_compile_goto(struct Opp_Context* opp, struct Opp_Node* lab)
{
	char* name = lab->goto_stmt.name->unary_expr.val.strval;
	struct Opp_Bucket* label = env_get_item(opp->info.cur_ns, name);

	opp_realloc_instrs(opp);
	opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_JMP;
	opp->ir.opcodes[opp->ir.instr_idx].jmp.type = PURE_JMP;

	if (label) {
		if (label->type != TYPE_LABEL)
			opp_compile_error(opp, lab,
				"Expected a label to jump to 'goto %s'", name);
		opp->ir.opcodes[opp->ir.instr_idx].jmp.loc = label->var.offset;
	}
	else {
		if (opp->info.goto_idx >= DEFAULT_OFFSET_TABLE) 
			opp_compile_error(opp, lab, "Max goto limit met (%d) in 'goto %s'",
				DEFAULT_OFFSET_TABLE, name);
		opp->info.goto_list[opp->info.goto_idx].label = name;
		opp->info.goto_list[opp->info.goto_idx].op = &opp->ir.opcodes[opp->ir.instr_idx];
		opp->info.goto_list[opp->info.goto_idx].goto_node = lab;
		opp->info.goto_idx++;
	}

	opp->ir.instr_idx++;
}

static void opp_compile_if(struct Opp_Context* opp, struct Opp_Node* ifstmt)
{
	opp_compile_time_eval(opp, ifstmt->if_stmt.cond);

	if (ifstmt->if_stmt.cond->type == EUNARY) {
		if (ifstmt->if_stmt.cond->unary_expr.val.i64val)
			opp_compile_stmt(opp, ifstmt->if_stmt.then);
		else {
			if (ifstmt->if_stmt.other != NULL)
				opp_compile_stmt(opp, ifstmt->if_stmt.other);
		}
		return;
	}

	bool iselse = ifstmt->if_stmt.other == NULL ? 0 : 1;

	opp->cond_state.cond_type = IFWHILE_COND;
	opp_check_label(opp, iselse ? 3 : 2);

	opp->cond_state.endloc = opp->info.label_loc++;
	opp->cond_state.locs[0] = opp->info.label_loc++;
	if (iselse) opp->cond_state.locs[1] = opp->info.label_loc++;
	else opp->cond_state.locs[1] = opp->cond_state.endloc;


	/* If structure
		<cond>
	true:
		true ...
	false:
		false ...
	end:

	*/

	opp_compile_expr(opp, ifstmt->if_stmt.cond);

	opp_realloc_instrs(opp);
	opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_LABEL;
	opp->ir.opcodes[opp->ir.instr_idx].constant.imm_u32 = opp->cond_state.locs[0];
	opp->ir.instr_idx++;

	opp_compile_stmt(opp, ifstmt->if_stmt.then);

	if (iselse) {
		opp_realloc_instrs(opp);
		opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_JMP;
		opp->ir.opcodes[opp->ir.instr_idx].jmp.type = PURE_JMP;
		opp->ir.opcodes[opp->ir.instr_idx].jmp.loc = opp->cond_state.endloc;
		opp->ir.instr_idx++;

		opp_realloc_instrs(opp);
		opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_LABEL;
		opp->ir.opcodes[opp->ir.instr_idx].constant.imm_u32 = opp->cond_state.locs[1];
		opp->ir.instr_idx++;
		opp_compile_stmt(opp, ifstmt->if_stmt.other);
	}

	opp_realloc_instrs(opp);
	opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_LABEL;
	opp->ir.opcodes[opp->ir.instr_idx].constant.imm_u32 = opp->cond_state.endloc;
	opp->ir.instr_idx++;
}

static void opp_compile_extern(struct Opp_Context* opp, struct Opp_Node* extrn)
{

}

static void opp_compile_while(struct Opp_Context* opp, struct Opp_Node* loop)
{
	// optimize while (1)
	// check for break inside, if no then allow for noreturn optimization
}

static void opp_compile_for(struct Opp_Context* opp, struct Opp_Node* loop)
{

}

static void opp_compile_switch(struct Opp_Context* opp, struct Opp_Node* cond)
{

}

static struct Opp_Type opp_compile_unary(struct Opp_Context* opp, struct Opp_Node* unary)
{
	struct Opp_Type type_info = {0};
	opp_realloc_instrs(opp);

	opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_CONST;
	opp->ir.opcodes[opp->ir.instr_idx].constant.nopush = 0;

	switch (unary->unary_expr.type)
	{
		case TINTEGER:
			opp->ir.opcodes[opp->ir.instr_idx].constant.type = IMM_I64;
			opp->ir.opcodes[opp->ir.instr_idx].constant.imm_i64 = unary->unary_expr.val.i64val;
			type_info.type = TYPE_NUM;
			break;

		case TSTR:
			opp->ir.opcodes[opp->ir.instr_idx].constant.type = IMM_STR;
			opp->ir.opcodes[opp->ir.instr_idx].constant.imm_sym = unary->unary_expr.val.strval;
			type_info.type = TYPE_PTR;
			type_info.ptr_depth = 1;
			break;

		case TIDENT:
			type_info = opp_compile_literal(opp, unary);
			break;

		default: break;
	}

	opp->ir.instr_idx++;

	return type_info;
}

static struct Opp_Type opp_compile_literal(struct Opp_Context* opp, struct Opp_Node* unary)
{
	struct Opp_Type type_info = {0};
	opp->ir.opcodes[opp->ir.instr_idx].constant.type = IMM_LOC;

	struct Opp_Bucket* sym = env_get_item(opp->info.cur_ns, unary->unary_expr.val.strval);

	if (sym == NULL)
		opp_compile_error(opp, unary, "Use of undefined variable '%s'", 
			unary->unary_expr.val.strval);

	if (sym->type == TYPE_LABEL)
		opp_compile_error(opp, unary, 
			"Invalid use of the variable '%s'", unary->unary_expr.val.strval);

	type_info.type = sym->var.type;
	if (sym->var.type == TYPE_PTR)
		type_info.ptr_depth = sym->var.ptr_depth;

	if (opp->info.cur_ns == opp->global_ns) {
		opp->ir.opcodes[opp->ir.instr_idx].constant.global = 1;
		printf("ADD GLOBAL\n");
	}
	else {
		opp->ir.opcodes[opp->ir.instr_idx].constant.global = 0;
		opp->ir.opcodes[opp->ir.instr_idx].constant.imm_i32 = sym->var.offset;
	}

	return type_info;
}

static struct Opp_Node* opp_compile_time_eval(struct Opp_Context* opp, struct Opp_Node* expr)
{
	struct Opp_Node* lhs = NULL;
	struct Opp_Node* rhs = NULL;

	switch (expr->type)
	{
		case EUNARY: {
			if (expr->unary_expr.type == TSTR)
				opp_compile_error(opp, expr,
					"Unexpected '%s' type in expression", expr->unary_expr.val.strval);
			return expr;
		}

		case EBIN: {
			lhs = opp_compile_time_eval(opp, expr->bin_expr.left);
			rhs = opp_compile_time_eval(opp, expr->bin_expr.right);
	
			if (lhs == NULL || rhs == NULL)
				return NULL;

			if (lhs->unary_expr.type == TINTEGER && rhs->unary_expr.type == TINTEGER) {
				const int op = expr->bin_expr.tok;
				struct Opp_Debug temp = {.line = expr->debug.line, .colum = expr->debug.colum};
				memset(expr, 0, sizeof(struct Opp_Node));

				expr->type = EUNARY;
				expr->unary_expr.type = TINTEGER;
				expr->debug = temp;

				switch (op)
				{
					case TADD: expr->unary_expr.val.i64val = 
							lhs->unary_expr.val.i64val + rhs->unary_expr.val.i64val;
						break;

					case TMIN: expr->unary_expr.val.i64val = 
							lhs->unary_expr.val.i64val - rhs->unary_expr.val.i64val;
						break;

					case TMUL:
						expr->unary_expr.val.i64val =
							lhs->unary_expr.val.i64val * rhs->unary_expr.val.i64val;
						break;

					case TDIV:
						expr->unary_expr.val.i64val =
							lhs->unary_expr.val.i64val / rhs->unary_expr.val.i64val;
						break;

					case TMOD:
						expr->unary_expr.val.i64val =
							lhs->unary_expr.val.i64val % rhs->unary_expr.val.i64val;
						break;
				}

				return expr;
			}

			break;
		}

		case ELOGIC: {
			lhs = opp_compile_time_eval(opp, expr->logic_expr.left);
			rhs = opp_compile_time_eval(opp, expr->logic_expr.right);
			
			if (lhs == NULL || rhs == NULL) 
				return NULL;

			if (lhs->unary_expr.type == TINTEGER && rhs->unary_expr.type == TINTEGER) {
				const int op = expr->logic_expr.tok;
				struct Opp_Debug temp = {.line = expr->debug.line, .colum = expr->debug.colum};
				memset(expr, 0, sizeof(struct Opp_Node));

				expr->type = EUNARY;
				expr->unary_expr.type = TINTEGER;
				expr->debug = temp;

				switch (op)
				{
					case TEQEQ:
						expr->unary_expr.val.i64val = 
							lhs->unary_expr.val.i64val == rhs->unary_expr.val.i64val;
						break;

					case TNOTEQ:
						expr->unary_expr.val.i64val = 
							lhs->unary_expr.val.i64val != rhs->unary_expr.val.i64val;
						break;

					case TLT:
						expr->unary_expr.val.i64val = 
							lhs->unary_expr.val.i64val < rhs->unary_expr.val.i64val;
						break;

					case TLE:
						expr->unary_expr.val.i64val = 
							lhs->unary_expr.val.i64val <= rhs->unary_expr.val.i64val;
						break;

					case TGT:
						expr->unary_expr.val.i64val = 
							lhs->unary_expr.val.i64val > rhs->unary_expr.val.i64val;
						break;

					case TGE:
						expr->unary_expr.val.i64val = 
							lhs->unary_expr.val.i64val >= rhs->unary_expr.val.i64val;
						break;

					case TOR:
						expr->unary_expr.val.i64val = 
							lhs->unary_expr.val.i64val || rhs->unary_expr.val.i64val;
						break;

					case TAND:
						expr->unary_expr.val.i64val = 
							lhs->unary_expr.val.i64val && rhs->unary_expr.val.i64val;
						break;
				}

				return expr;
			}

			break;
		}
		default: break;
	}
	return NULL;
}

static struct Opp_Type opp_compile_bin(struct Opp_Context* opp, struct Opp_Node* bin)
{
	opp_compile_time_eval(opp, bin);
	struct Opp_Type lhs;
	struct Opp_Type rhs;
	bool imm = false;

	if (bin->type == EUNARY) {
		lhs = opp_compile_unary(opp, bin);
		return lhs;
	}

	// Find first bin side TODO: MOVE TO FN??
	// This needs to also make consts on the rhs
	if (bin->bin_expr.right->type == ECALL) {
		struct Opp_Node* temp = bin->bin_expr.left;
		bin->bin_expr.left = bin->bin_expr.right;
		bin->bin_expr.right = temp;
	}

	lhs = opp_compile_expr(opp, bin->bin_expr.left);


	// SWITCH THIS INTO DOING const expr eval and if its unary
	if (bin->bin_expr.right->type == EUNARY 
		&& bin->bin_expr.right->unary_expr.type == TINTEGER
		&& bin->bin_expr.right->unary_expr.val.i64val <= UINT_MAX)
	{
		imm = true;
		rhs.type = TYPE_NUM;
	}
	else
		rhs = opp_compile_expr(opp, bin->bin_expr.right);

	if (lhs.type != rhs.type)
		opp_warning(opp, bin, 
			"Found mixing types in a binary operation");

	opp_realloc_instrs(opp);
	opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_ARITH;
	opp->ir.opcodes[opp->ir.instr_idx].arith.imm = 0;
	opp->ir.opcodes[opp->ir.instr_idx].arith.type = bin->bin_expr.tok;

	if (imm) {
		opp->ir.opcodes[opp->ir.instr_idx].arith.imm = 1;
		opp->ir.opcodes[opp->ir.instr_idx].arith.val.type = IMM_I8;
		if (bin->bin_expr.right->unary_expr.val.i64val > 255)
			opp->ir.opcodes[opp->ir.instr_idx].arith.val.type = IMM_I32;
		opp->ir.opcodes[opp->ir.instr_idx].arith.val.imm_i32 = 
			(uint32_t)bin->bin_expr.right->unary_expr.val.i64val;
	}
	opp->ir.instr_idx++;

	return lhs;
}

static struct Opp_Type opp_compile_logic(struct Opp_Context* opp, struct Opp_Node* logic)
{
	struct Opp_Type type_info;
	bool in_parent = false;

	if (!opp->cond_state.in_logic) {
		switch (opp->cond_state.cond_type)
		{
			case LOGIC_COND: {
				opp_compile_time_eval(opp, logic);

				if (logic->type == EUNARY) {
					type_info = opp_compile_unary(opp, logic);
					return type_info;
				}

				opp_check_label(opp, 3);
				opp->cond_state.endloc = opp->info.label_loc++;
				opp->cond_state.locs[0] = opp->info.label_loc++; // True
				opp->cond_state.locs[1] = opp->info.label_loc++; // False

				opp->cond_state.use_op = true;
				opp->cond_state.jloc = opp->cond_state.locs[1];
				opp->cond_state.curr_logic = 0;
				opp->cond_state.hs = 0;
				break;
			}

			case IFWHILE_COND: {
				opp->cond_state.use_op = true;
				opp->cond_state.jloc = opp->cond_state.locs[1];
				opp->cond_state.curr_logic = 0;
				opp->cond_state.hs = 0;
				break;
			}

			case SWITCH_COND: { break; }
		}
		opp->cond_state.in_logic = true;
		in_parent = true;
	}

	type_info = opp_compile_logic_assign(opp, logic);

	if (in_parent) {
		opp_compile_logic_end(opp, logic);
		opp->cond_state.in_logic = false;
	}

	return type_info;
}

static void opp_compile_logic_end(struct Opp_Context* opp, struct Opp_Node* logic)
{
	switch (opp->cond_state.cond_type) 
	{
		case LOGIC_COND: {
			opp_realloc_instrs(opp);
			opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_LABEL;
			opp->ir.opcodes[opp->ir.instr_idx].constant.imm_u32 = opp->cond_state.locs[0];
			opp->ir.instr_idx++;

			opp_realloc_instrs(opp);
			opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_CONST;
			opp->ir.opcodes[opp->ir.instr_idx].constant.type = IMM_I64;
			opp->ir.opcodes[opp->ir.instr_idx].constant.imm_i64 = 1;
			opp->ir.opcodes[opp->ir.instr_idx].constant.nopush = 1;
			opp->ir.instr_idx++;

			opp_realloc_instrs(opp);
			opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_JMP;
			opp->ir.opcodes[opp->ir.instr_idx].jmp.type = PURE_JMP;
			opp->ir.opcodes[opp->ir.instr_idx].jmp.loc = opp->cond_state.endloc;
			opp->ir.instr_idx++;

			opp_realloc_instrs(opp);
			opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_LABEL;
			opp->ir.opcodes[opp->ir.instr_idx].constant.imm_u32 = opp->cond_state.locs[1];
			opp->ir.instr_idx++;

			opp_realloc_instrs(opp);
			opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_CONST;
			opp->ir.opcodes[opp->ir.instr_idx].constant.type = IMM_I64;
			opp->ir.opcodes[opp->ir.instr_idx].constant.imm_i64 = 0;
			opp->ir.opcodes[opp->ir.instr_idx].constant.nopush = 0;
			opp->ir.instr_idx++;

			opp_realloc_instrs(opp);
			opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_LABEL;
			opp->ir.opcodes[opp->ir.instr_idx].constant.imm_u32 = opp->cond_state.endloc;
			opp->ir.instr_idx++;
			break;
		}

		case IFWHILE_COND:
			opp->cond_state.cond_type = LOGIC_COND;
			break;

		case SWITCH_COND:
			break;
	}
}

static enum Opp_Token jmp_opposites[] = {
	[TGT] = TLE,
	[TLT] = TGE,
	[TEQEQ] = TNOTEQ,
	[TGE] = TLT,
	[TLE] = TGT,
	[TNOTEQ] = TEQEQ
};

static struct Opp_Type opp_compile_logic_assign(struct Opp_Context* opp, struct Opp_Node* logic)
{
	struct Opp_Type type_info = {0};
	struct Opp_Cond save = opp->cond_state;

	switch (logic->logic_expr.tok) 
	{
		case TEQEQ: case TNOTEQ: case TLT:
		case TGT: case TLE: case TGE: {
			struct Opp_Type lhs = opp_compile_expr(opp, logic->logic_expr.left);
			struct Opp_Type rhs = opp_compile_expr(opp, logic->logic_expr.right);

			if (lhs.type != rhs.type)
				opp_warning(opp, logic, "Comparing different types");
			// TODO: check for imm

			opp_realloc_instrs(opp);
			opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_CMP;
			opp->ir.opcodes[opp->ir.instr_idx].cmp.imm = 0;
			opp->ir.instr_idx++;

			opp_realloc_instrs(opp);
			opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_JMP;

			if (opp->cond_state.use_op)
				opp->ir.opcodes[opp->ir.instr_idx].jmp.type = 
					jmp_opposites[logic->logic_expr.tok];
			else
				opp->ir.opcodes[opp->ir.instr_idx].jmp.type = 
					logic->logic_expr.tok;

			opp->ir.opcodes[opp->ir.instr_idx].jmp.loc = opp->cond_state.jloc;
			opp->ir.instr_idx++;
			break;
		}

		case TOR: {
			opp->cond_state.curr_logic = TOR;

			if (logic->logic_expr.left->logic_expr.tok == TAND) {
				opp_check_label(opp, 1);
				opp->cond_state.locs[1] = opp->info.label_loc++;
			}

			opp->cond_state.use_op = 0;
			opp->cond_state.jloc = opp->cond_state.locs[0];
			opp->cond_state.hs = 0;
			opp_compile_logic_assign(opp, logic->logic_expr.left); // Make possibly for assuming true Ex. 1 && 1

			if (logic->logic_expr.left->logic_expr.tok == TAND) {
				opp_realloc_instrs(opp);
				opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_LABEL;
				opp->ir.opcodes[opp->ir.instr_idx].constant.imm_u32 = opp->cond_state.locs[1];
				opp->ir.instr_idx++;

				opp->cond_state.locs[1] = save.locs[1];
			}

			if (save.curr_logic == TOR && save.hs == 0) {
				opp->cond_state.use_op = 0;
				opp->cond_state.jloc = opp->cond_state.locs[0];
			}
			else if (save.hs == 1 && save.curr_logic == TOR) {
				opp->cond_state.use_op = 0;
				opp->cond_state.jloc = opp->cond_state.locs[0];
			}
			else {
				opp->cond_state.use_op = 1;
				opp->cond_state.jloc = opp->cond_state.locs[1];
			}
			opp->cond_state.hs = 1;
			opp->cond_state.curr_logic = save.curr_logic;
			opp_compile_logic_assign(opp, logic->logic_expr.right);
			break;
		}

		case TAND: {
			opp->cond_state.curr_logic = TAND;

			if (logic->logic_expr.left->logic_expr.tok == TOR) {
				opp_check_label(opp, 1);
				opp->cond_state.locs[0] = opp->info.label_loc++;
			}

			opp->cond_state.use_op = 1;
			opp->cond_state.jloc = opp->cond_state.locs[1];
			opp->cond_state.hs = 0;
			opp_compile_logic_assign(opp, logic->logic_expr.left); // Make possibly for assuming true Ex. 1 && 1

			if (logic->logic_expr.left->logic_expr.tok == TOR) {
				opp_realloc_instrs(opp);
				opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_LABEL;
				opp->ir.opcodes[opp->ir.instr_idx].constant.imm_u32 = opp->cond_state.locs[0];
				opp->ir.instr_idx++;

				opp->cond_state.locs[0] = save.locs[0];
			}

			if (save.curr_logic == TOR && save.hs == 0) {
				opp->cond_state.use_op = 0;
				opp->cond_state.jloc = opp->cond_state.locs[0];
			}
			else if (save.hs == 1 && save.curr_logic == TOR) {
				opp->cond_state.use_op = 0;
				opp->cond_state.jloc = opp->cond_state.locs[0];
			}
			else {
				opp->cond_state.use_op = 1;
				opp->cond_state.jloc = opp->cond_state.locs[1];
			}
			opp->cond_state.hs = 1;
			opp->cond_state.curr_logic = save.curr_logic;
			opp_compile_logic_assign(opp, logic->logic_expr.right);
			break;
		}

		default: break;
	}

	opp->cond_state = save;
	return type_info;
}

static struct Opp_Type opp_compile_assign(struct Opp_Context* opp, struct Opp_Node* assign)
{
	struct Opp_Node* node = assign->assign_expr.ident;

	if ((assign->assign_expr.ident->type != EUNARY 
		&& assign->assign_expr.ident->unary_expr.type != TIDENT)
	 	&& assign->assign_expr.ident->type != EDEREF)
	 	opp_compile_error(opp, assign,
			"Expected a identifier on the left side of '=' operator");

	if (node->type == EDEREF)
		return opp_compile_ptr_assign(opp, assign);

	struct Opp_Bucket* var = env_get_item(opp->info.cur_ns, node->unary_expr.val.strval);

	if (var == NULL)
		opp_compile_error(opp, assign,
			"Attempt to assign a undeclared var '%s'", node->unary_expr.val.strval);

	if (var->type == TYPE_LABEL)
		opp_compile_error(opp, assign,
			"Attempt to assign a '%s' which is not a variable", node->unary_expr.val.strval);

	struct Opp_Type rhs = opp_compile_expr(opp, assign->assign_expr.val);

	if (rhs.type != var->var.type)
		opp_warning(opp, assign,
			"Attempt to assign var '%s' to not the same type");
	if (rhs.type == TYPE_PTR && rhs.ptr_depth > var->var.ptr_depth)
		opp_warning(opp, assign,
			"Attempt to assign var '%s' to a pointer of depth '%d'", rhs.ptr_depth);

	opp_realloc_instrs(opp);
	opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_ASSIGN;
	opp->ir.opcodes[opp->ir.instr_idx].set.global = 0;
	opp->ir.opcodes[opp->ir.instr_idx].set.imm = 0; // TODO: OPTIMIZE IMM

	if (var->type == TYPE_GLOBAL) {
		opp->ir.opcodes[opp->ir.instr_idx].set.global = 1;
		opp->ir.opcodes[opp->ir.instr_idx].set.val.type = IMM_STR;
		// opp->ir.opcodes[opp->ir.instr_idx].set.val.imm_sym = var->key; // TODO MAKE COPY ?? 
	}
	else {
		opp->ir.opcodes[opp->ir.instr_idx].set.val.type = IMM_LOC;
		opp->ir.opcodes[opp->ir.instr_idx].set.val.imm_i32 = var->var.offset;
	}

	opp->ir.instr_idx++;

	return rhs;
}

static struct Opp_Type opp_compile_ptr_assign(struct Opp_Context* opp, struct Opp_Node* expr)
{
	struct Opp_Type type_info = {0};
	unsigned int ptr_depth = 0;
	struct Opp_Node* node = expr->assign_expr.ident;

	while (node->type != EUNARY && node->type != EBIN) {
		if (node->type != EDEREF)
			opp_compile_error(opp, expr,
				"Error in dereferance assigment (unexpected lvalue)");
		node = node->defer_expr.defer;
		ptr_depth++;
	}

	// set a flag 
	// ex
	/*
		***a = 3;

		into deref # 1
		into deref # 2
		into deref # 3
			Sees flag and ident / bin next

		mov rax, [a]
		mov rcx, [rax]
		mov rax, [rcx]
		mov rcx, 2
		mov [rax], rcx

	*/


	return type_info;
}

static struct Opp_Type opp_compile_sub(struct Opp_Context* opp, struct Opp_Node* sub)
{
	struct Opp_Type type_info = {0};

	if (sub->sub_expr.unary->type == EUNARY && sub->sub_expr.unary->unary_expr.type != TIDENT) {
		if (sub->sub_expr.unary->unary_expr.type != TINTEGER)
			opp_warning(opp, sub,
				"Attempting to negate a non integer type");

		int64_t neg = -sub->sub_expr.unary->unary_expr.val.i64val;
		struct Opp_Debug temp = {.line = sub->debug.line, .colum = sub->debug.colum};
		memset(sub, 0, sizeof(struct Opp_Node));

		sub->type = EUNARY;
		sub->unary_expr.type = TINTEGER;
		sub->unary_expr.val.i64val = neg;
		sub->debug = temp;
		type_info.type = TYPE_NUM;
		return type_info;
	}

	opp_realloc_instrs(opp);
	opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_ARITH;
	// add mul -1
	opp->ir.instr_idx++;

	return type_info;
}

static struct Opp_Type opp_compile_call(struct Opp_Context* opp, struct Opp_Node* call)
{
	struct Opp_Type type_info = {0};

	opp_realloc_instrs(opp);
	opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_CALL;
	opp->ir.instr_idx++;

	return type_info;
}

static struct Opp_Type opp_compile_addr(struct Opp_Context* opp, struct Opp_Node* expr)
{
	struct Opp_Type type_info = {0};
	return type_info;
}

static struct Opp_Type opp_compile_deref(struct Opp_Context* opp, struct Opp_Node* expr)
{
	struct Opp_Type type_info = {0};
	return type_info;
}