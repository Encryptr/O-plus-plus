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
	opp->oppir = NULL;

	// Info
	memset(&opp->info, 0, sizeof(struct Opp_Info));
	opp->info.sym_loc = 4; 

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

static Opp_Type opp_type_to_ir(struct Opp_Type_Decl* type)
{
	Opp_Type ret_type = 0;
	bool sign = type->unsign;

	if (type->depth > 0 || type->size != 0)
		return IMM_U64;

	switch (type->decl->t_type)
	{
		case TYPE_I8:  ret_type = sign ? IMM_U8: IMM_I8; break;
		case TYPE_I16: ret_type = sign ? IMM_U16: IMM_I16; break;
		case TYPE_I32: ret_type = sign ? IMM_U32: IMM_I32; break;
		case TYPE_I64: ret_type = sign ? IMM_U64: IMM_I64; break; 
		case TYPE_FLOAT: ret_type = IMM_F32; break;
		case TYPE_DOUBLE: ret_type = IMM_F64; break;
		
		default:
		// 	printf("Error '%s'\n", __FUNCTION__);
			break;
	}

	return ret_type;
}

static unsigned int opp_type_to_size(struct Opp_Type_Decl* type)
{
	unsigned int size = 0;

	if (type->depth > 0 && type->size == 0)
		return 8;
	else if (type->size > 0)
		return type->size;

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
		
		default:
			printf("Error '%s'\n", __FUNCTION__);
			break;
	}

	return size;
}

static void opp_try_cast(struct Opp_Context* opp, Opp_Type lhs, Opp_Type rhs)
{
	if ((lhs >= IMM_F32 || rhs >= IMM_F32) && (lhs != rhs)) {
		printf("NEED TO CAST\n");
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

			case STMT_STRUCT:
				opp_compile_struct(opp, opp->parser->statments[stmt]);
				break;

			case STMT_EXTERN:
				opp_compile_extern(opp, opp->parser->statments[stmt]);
				break;

			// case STMT_IMPORT:
			// 	opp_import_module(opp, opp->parser->statments[stmt]);
			// 	break;

			case STMT_VAR:
				opp_compile_var(opp, opp->parser->statments[stmt]);
				break;

			default: break;
		}
	}
}

static void opp_compile_struct(struct Opp_Context* opp, struct Opp_Node* node)
{
	struct Opp_Type_Entry* b = get_type(&opp->parser->tree, node->struct_stmt.name);
	unsigned int size = 0;

	for (unsigned int i = 0; i < node->struct_stmt.len; i++)
		size += opp_type_to_size(&node->struct_stmt.elems[i]->var_stmt.type);

	b->size = size;
}

static void opp_compile_args(struct Opp_Context* opp, struct Opp_Node* func)
{
	unsigned int Fidx = 0, Iidx = 0;

	for (unsigned int i = 0; i < func->fn_stmt.len; i++)
	{
		opp_realloc_instrs(opp);
			opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_ARG;

			if (func->fn_stmt.args[i].var_stmt.type.decl->t_type == TYPE_STRUCT) {
				printf("UNSUPPORTED FOR NOW\n");
				continue;
			}

			opp->ir.opcodes[opp->ir.instr_idx].arg.type = 
				opp_type_to_ir(&func->fn_stmt.args[i].var_stmt.type);

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

		opp->ir.instr_idx++;

		opp_compile_var(opp, &func->fn_stmt.args[i]);
	}
}

static void opp_compile_func(struct Opp_Context* opp, struct Opp_Node* func)
{
	// Resets
	opp->info.stack_offset = 0;
	opp->info.label_loc = 1;

	struct Opp_Bucket* b = env_get_item(opp->env.global_ns, func->fn_stmt.name->unary_expr.val.strval);
	b->offset = opp->info.sym_loc++;

	opp_realloc_instrs(opp);
		opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_FUNC;
		opp->ir.opcodes[opp->ir.instr_idx].func.fn_name = func->fn_stmt.name->unary_expr.val.strval;
		opp->ir.opcodes[opp->ir.instr_idx].func.sym = b->offset;
	opp->ir.instr_idx++;

	struct Opp_Namespace* temp = opp->env.curr_ns;
	opp->env.curr_ns = func->fn_stmt.scope;

	opp_compile_args(opp, func);

	for (unsigned int i = 0; i < func->fn_stmt.body->block_stmt.len; i++)
		opp_compile_stmt(opp, func->fn_stmt.body->block_stmt.stmts[i]);

	opp->env.curr_ns = temp;
	// Generate end label

	opp_realloc_instrs(opp);
		opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_END;
	opp->ir.instr_idx++;
}

static void opp_compile_extern(struct Opp_Context* opp, struct Opp_Node* extrn)
{

}

static void opp_compile_stmt(struct Opp_Context* opp, struct Opp_Node* stmt)
{
	switch (stmt->type)
	{
		case IGNORE: break;

		case STMT_RET:
			opp_compile_ret(opp, stmt);
			break;
		// case STMT_LABEL:
		// 	opp_compile_label(opp, stmt);
		// 	break;

		// case STMT_GOTO:
		// 	opp_compile_goto(opp, stmt);
		// 	break;

		// case STMT_IF:
		// 	opp_compile_if(opp, stmt);
		// 	break;

		// case STMT_WHILE:
		// 	opp_compile_while(opp, stmt);
		// 	break;

		// case STMT_FOR:
		// case STMT_SWITCH:
		// 	break;

		case STMT_VAR:
			opp_compile_var(opp, stmt);
			break;

		// case STMT_BLOCK:
		// 	opp_compile_block(opp, stmt);
		// 	break;

		case EASSIGN:
			opp_compile_assign(opp, stmt);
			break;

		default: 
			opp_compile_expr(opp, stmt);
			break;
	}
}

static void opp_compile_ret(struct Opp_Context* opp, struct Opp_Node* ret)
{
	// need to know function ret type
}

static void opp_compile_var(struct Opp_Context* opp, struct Opp_Node* var)
{
	bool assign = var->var_stmt.var->type == EASSIGN ? 1 : 0;

	struct Opp_Bucket* b;

	if (assign)
		b = env_get_item(opp->env.curr_ns, var->var_stmt.var->assign_expr.ident->unary_expr.val.strval);
	else 
		b = env_get_item(opp->env.curr_ns, var->var_stmt.var->unary_expr.val.strval);

	opp_realloc_instrs(opp);
		opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_VAR;
		opp->ir.opcodes[opp->ir.instr_idx].var.global = opp->env.curr_ns == opp->env.global_ns;
		opp->ir.opcodes[opp->ir.instr_idx].var.size = opp_type_to_size(&var->var_stmt.type);
		opp->ir.opcodes[opp->ir.instr_idx].var.name = b->key;
	opp->ir.instr_idx++;

	if (opp->env.curr_ns == opp->env.global_ns)
		b->offset = opp->info.sym_loc++;
	else {
		opp->info.stack_offset -= opp->ir.opcodes[opp->ir.instr_idx - 1].var.size;
		b->offset = opp->info.stack_offset;
	}

	if (assign)
		opp_compile_expr(opp, var->var_stmt.var);
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

static void opp_compile_while(struct Opp_Context* opp, struct Opp_Node* loop)
{

}

static void opp_compile_for(struct Opp_Context* opp, struct Opp_Node* loop)
{

}

static void opp_compile_switch(struct Opp_Context* opp, struct Opp_Node* cond)
{

}

static Opp_Type opp_compile_expr(struct Opp_Context* opp, struct Opp_Node* expr)
{
	switch (expr->type)
	{
		case EASSIGN:
			return opp_compile_assign(opp, expr);

		case EUNARY:
			return opp_compile_unary(opp, expr);

		case EDOT:
			return opp_compile_dot(opp, expr);

		default: break;
	}
}

static Opp_Type opp_compile_unary(struct Opp_Context* opp, struct Opp_Node* unary)
{
	Opp_Type ret_type = 0;
	opp_realloc_instrs(opp);
	opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_CONST;
	switch (unary->unary_expr.type)
	{
		case TINTEGER: { 
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_I64;
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i64 = unary->unary_expr.val.i64val;
			ret_type = IMM_I64;
			break;
		}

		case TFLOAT: {
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_F64;
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_f64 = unary->unary_expr.val.f64val;
			ret_type = IMM_F64;
			break;
		}

		case TIDENT: {
			struct Opp_Bucket* b = env_get_item(opp->env.curr_ns, unary->unary_expr.val.strval);

			if (b->sym_type.decl->t_type == TYPE_STRUCT) {
				opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_ADDR;
				opp->info.dot.s_ns = b->sym_type.decl->s_elems;
			}

			if (b->type == TYPE_GLOBAL || b->type == TYPE_FUNC)  {
				opp->ir.opcodes[opp->ir.instr_idx].constant.global = 1;
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_SYM;
			}
			else {
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_LOC;
			}
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = b->offset;
			ret_type = opp_type_to_ir(&b->sym_type);
			opp->ir.opcodes[opp->ir.instr_idx].constant.loc_type = ret_type;
			break;
		}

		case TSTR: {
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_STR;
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_sym = unary->unary_expr.val.strval;
			ret_type = IMM_U64;
			break;
		}
	}
	opp->ir.instr_idx++;

	return ret_type;
}

static Opp_Type opp_compile_dot(struct Opp_Context* opp, struct Opp_Node* dot)
{
	bool outer = false;
	if (!opp->info.dot.indot) {
		opp->info.dot.indot = true;
		outer = true;
	}
	opp_compile_expr(opp, dot->dot_expr.left);

	unsigned int off = 0;
	struct Opp_Stmt_Var* v = NULL;
	for (unsigned int i = opp->info.dot.s_ns->len; i > 0; i--) {
		if (!strcmp(dot->dot_expr.right->unary_expr.val.strval,
			opp->info.dot.s_ns->elems[i-1]->var_stmt.var->unary_expr.val.strval))
		{
			v = &opp->info.dot.s_ns->elems[i-1]->var_stmt;
			if (v->type.decl->t_type == TYPE_STRUCT)
				opp->info.dot.s_ns = v->type.decl->s_elems;
			break;
		}
		off += opp_type_to_size(&opp->info.dot.s_ns->elems[i-1]->var_stmt.type);
	}

	if (off > 0) {
		opp_realloc_instrs(opp);
			opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_ARITH;
			opp->ir.opcodes[opp->ir.instr_idx].arith.type = TADD;
			opp->ir.opcodes[opp->ir.instr_idx].arith.imm = 1;
			opp->ir.opcodes[opp->ir.instr_idx].arith.val.type = (off > 255) 
				? IMM_I8 : IMM_I32;
			opp->ir.opcodes[opp->ir.instr_idx].arith.val.imm_i32 = (int32_t)off;
		opp->ir.instr_idx++;
	}

	if (outer && !opp->info.in_assign) {
		opp_realloc_instrs(opp);
			opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_DEREF;
			opp->ir.opcodes[opp->ir.instr_idx].cast.type = opp_type_to_ir(&v->type);
		opp->ir.instr_idx++;
		opp->info.dot.indot = 0;
	}
	return opp_type_to_ir(&v->type);
}

static Opp_Type opp_compile_bin(struct Opp_Context* opp, struct Opp_Node* bin)
{

}

static Opp_Type opp_compile_sub(struct Opp_Context* opp, struct Opp_Node* sub)
{

}

static Opp_Type opp_compile_dot_assign(struct Opp_Context* opp, struct Opp_Node* expr)
{
	opp->info.in_assign = 1;
	Opp_Type lhs = opp_compile_expr(opp, expr->assign_expr.ident);
	opp->info.in_assign = 0;

	Opp_Type rhs = opp_compile_expr(opp, expr->assign_expr.val);

	// CAST 
	opp_try_cast(opp, lhs, rhs);

	opp_realloc_instrs(opp);
		opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_PTR_ASSIGN;
		opp->ir.opcodes[opp->ir.instr_idx].cast.type = lhs;
	opp->ir.instr_idx++;
	return lhs;
}

static Opp_Type opp_compile_ptr_assign(struct Opp_Context* opp, struct Opp_Node* expr)
{
	
}

static Opp_Type opp_compile_assign(struct Opp_Context* opp, struct Opp_Node* assign)
{
	if (assign->assign_expr.ident->type == EDEREF || assign->assign_expr.ident->type == EELEMENT)
		return opp_compile_ptr_assign(opp, assign);
	if (assign->assign_expr.ident->type == EDOT)
		return opp_compile_dot_assign(opp, assign);

	struct Opp_Bucket* bucket = env_get_item(opp->env.curr_ns, 
		assign->assign_expr.ident->unary_expr.val.strval);

	Opp_Type rhs = opp_compile_expr(opp, assign->assign_expr.val);
	Opp_Type lhs = opp_type_to_ir(&bucket->sym_type);

	// opp_try_cast(opp, lhs, rhs);
	// if (rhs != lhs)
	// 	rhs = lhs;

	opp_realloc_instrs(opp);
		opp->ir.opcodes[opp->ir.instr_idx].type = OPCODE_ASSIGN;
		opp->ir.opcodes[opp->ir.instr_idx].set.loc_type = lhs;
		opp->ir.opcodes[opp->ir.instr_idx].set.global = 
			bucket->type == TYPE_GLOBAL || bucket->type == TYPE_EXTERN;
		if (opp->ir.opcodes[opp->ir.instr_idx].set.global)
			opp->ir.opcodes[opp->ir.instr_idx].set.val.type = IMM_SYM;
		else
			opp->ir.opcodes[opp->ir.instr_idx].set.val.type = IMM_I32;
		opp->ir.opcodes[opp->ir.instr_idx].set.val.imm_i32 = bucket->offset;
	opp->ir.instr_idx++;
}

static Opp_Type opp_compile_call(struct Opp_Context* opp, struct Opp_Node* call)
{

}

static Opp_Type opp_compile_logic(struct Opp_Context* opp, struct Opp_Node* logic)
{

}

static Opp_Type opp_compile_logic_assign(struct Opp_Context* opp, struct Opp_Node* logic)
{

}

static void opp_compile_logic_end(struct Opp_Context* opp, struct Opp_Node* logic)
{

}

static Opp_Type opp_compile_literal(struct Opp_Context* opp, struct Opp_Node* unary)
{

}

static Opp_Type opp_compile_addr(struct Opp_Context* opp, struct Opp_Node* expr)
{

}

static Opp_Type opp_compile_deref(struct Opp_Context* opp, struct Opp_Node* expr)
{

}