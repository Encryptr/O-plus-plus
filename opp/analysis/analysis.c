/** @file analysis.c
 * 
 * @brief Preform analysis stage
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

#include "analysis.h"

static unsigned int get_type_size(struct Opp_Type_Decl* type);
static struct Opp_Node* opp_analize_time_eval(struct Opp_Analize* ctx, struct Opp_Node* expr);

// Globals
static void opp_analize_global(struct Opp_Analize* ctx, struct Opp_Node* node);
static void opp_analize_func(struct Opp_Analize* ctx, struct Opp_Node* fn);
static void opp_analize_args(struct Opp_Analize* ctx, struct Opp_Node* fn, struct Opp_Bucket* bucket);
static void opp_analize_struct(struct Opp_Analize* ctx, struct Opp_Node* node);
static void opp_analize_extern(struct Opp_Analize* ctx, struct Opp_Node* node);

// Stmt
static void opp_analize_stmt(struct Opp_Analize* ctx, struct Opp_Node* node);
static void opp_analize_var(struct Opp_Analize* ctx, struct Opp_Node* node);
static void opp_analize_ret(struct Opp_Analize* ctx, struct Opp_Node* node);
static void opp_analize_block(struct Opp_Analize* ctx, struct Opp_Node* node);
static void opp_analize_if(struct Opp_Analize* ctx, struct Opp_Node* node);
static void opp_analize_while(struct Opp_Analize* ctx, struct Opp_Node* node);

// Expr
static Opp_Obj opp_analize_expr(struct Opp_Analize* ctx, struct Opp_Node* node);
static Opp_Obj opp_analize_unary(struct Opp_Analize* ctx, struct Opp_Node* node);
static Opp_Obj opp_analize_bin(struct Opp_Analize* ctx, struct Opp_Node* node);
static Opp_Obj opp_analize_logic(struct Opp_Analize* ctx, struct Opp_Node* node);
static Opp_Obj opp_analize_assign(struct Opp_Analize* ctx, struct Opp_Node* node);
static Opp_Obj opp_analize_call(struct Opp_Analize* ctx, struct Opp_Node* node);
static Opp_Obj opp_analize_dot(struct Opp_Analize* ctx, struct Opp_Node* node);
static Opp_Obj opp_analize_deref(struct Opp_Analize* ctx, struct Opp_Node* node);
static Opp_Obj opp_analize_sub(struct Opp_Analize* ctx, struct Opp_Node* node);
static Opp_Obj opp_analize_elem(struct Opp_Analize* ctx, struct Opp_Node* node);
static Opp_Obj opp_analize_addr(struct Opp_Analize* ctx, struct Opp_Node* node);
static Opp_Obj opp_analize_adjust(struct Opp_Analize* ctx, struct Opp_Node* node);
static Opp_Obj opp_analize_sizeof(struct Opp_Analize* ctx, struct Opp_Node* node);

struct Opp_Analize* opp_init_analize(struct Opp_Parser* parser, struct Opp_Options* opts)
{
	struct Opp_Analize* ctx = (struct Opp_Analize*)malloc(sizeof(struct Opp_Analize));

	if (ctx == NULL)
		goto err;

	ctx->opts = opts;

	ctx->parser = parser;
	ctx->debug.scope = NULL;
	ctx->debug.ret = 0;

	// Namespace
	ctx->ns = init_namespace(NULL, (void*)alloc);
	ctx->curr_ns = ctx->ns;
	ctx->parser->scope = ctx->ns;

	return ctx;

err:
	INTERNAL_ERROR("Malloc Fail");
	return NULL;
}

void opp_free_analize(struct Opp_Analize* analize)
{
	free(analize);
}

static void opp_semantics_error(struct Opp_Analize* ctx, 
		struct Opp_Node* node, const char* str, ...)
{
	va_list ap;
	va_start(ap, str);
	
	printf("%s:%u:%u ", ctx->parser->lex->io.fname, 
			node->debug.line, node->debug.colum);
	#ifdef UNX
		printf(CL_RED);
	#endif

	if (ctx->debug.scope != NULL)
		printf("semantics error: in function \"%s(...)\"\n\t", ctx->debug.scope);
	else
		printf("semantics error: in declaration\n\t");

	#ifdef UNX
		printf(CL_RESET);
	#endif
	vfprintf(stdout, str, ap);
	fprintf(stdout, "\n");
	va_end(ap);
	exit(1);
}

static void opp_warning(struct Opp_Analize* ctx, 
		struct Opp_Node* node, const char* str, ...)
{
	if (!ctx->opts->warning)
		return;

	if (ctx->opts->wall) {
		opp_semantics_error(ctx, node, str);
		return;
	}

	va_list ap;
	va_start(ap, str);
	
	printf("%s:%u:%u ", ctx->parser->lex->io.fname, 
			node->debug.line, node->debug.colum);
	#ifdef UNX
		printf(CL_YELLOW);
	#endif

	if (ctx->debug.scope != NULL)
		printf("warning: in function \"%s(...)\"\n\t", ctx->debug.scope);
	else 
		printf("warning: in declaration\n\t");

	#ifdef UNX
		printf(CL_RESET);
	#endif 
	vfprintf(stdout, str, ap);
	fprintf(stdout, "\n");
	va_end(ap);
}

static unsigned int get_type_size(struct Opp_Type_Decl* type)
{
	if (type->depth > 0 && type->size == 0)
		return 8;

	// TODO: IS THIS NEEDED????
	// if (type->size > 0)
	// 	return type->size;

	switch (type->decl->t_type)
	{
		case TYPE_VOID:   return 0;
		case TYPE_I8:     return 1;
		case TYPE_I16:    return 2;
		case TYPE_I32:    return 4;
		case TYPE_I64:    return 8;
		case TYPE_FLOAT:  return 4;
		case TYPE_DOUBLE: return 8;
		case TYPE_STRUCT: return type->decl->size;
		default: break;
	}

	return 0;
}

void analize_tree(struct Opp_Analize* ctx)
{
	for (unsigned int i = 0; i < ctx->parser->nstmts; i++)
		opp_analize_global(ctx, ctx->parser->statments[i]);
}

static void opp_analize_global(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	switch (node->type)
	{
		case STMT_FUNC:
			opp_analize_func(ctx, node);
			break;

		case STMT_EXTERN:
			opp_analize_extern(ctx, node);
			break;

		case STMT_VAR:
			opp_analize_var(ctx, node);
			break;

		case STMT_STRUCT:
			opp_analize_struct(ctx, node);
			break;

		case STMT_IMPORT:
			break;

		default: break;
	}
}

static void opp_analize_extern(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	struct Opp_Bucket* b;
	opp_analize_global(ctx, node->extrn_stmt.stmt);

	b = env_get_item(ctx->curr_ns, (char*)ctx->debug.res_name);

	assert(b != NULL);

	b->predef = false;
	b->type = TYPE_EXTERN;
}

static void opp_analize_func(struct Opp_Analize* ctx, struct Opp_Node* fn)
{
	struct Opp_Stmt_Func* func = &fn->fn_stmt;

	struct Opp_Bucket* bucket = env_add_item(ctx->curr_ns, func->name->unary_expr.val.strval);

	if (bucket == NULL) {
		bucket = env_get_item(ctx->curr_ns, func->name->unary_expr.val.strval);

		if (bucket == NULL || bucket->predef == false)
			opp_semantics_error(ctx, fn, "Redeclaration of function '%s'", func->name->unary_expr.val.strval);
	}

	bucket->type = TYPE_FUNC;
	bucket->sym_type = func->type;
	bucket->args = &fn->fn_stmt;
	bucket->predef = false;
	// idx
	func->bidx = bucket;

	if (func->type.decl->t_type == TYPE_STRUCT)
		opp_semantics_error(ctx, fn, "Cannot return struct from function '%s'", bucket->key);

	ctx->debug.res_name = func->name->unary_expr.val.strval;

	if (func->body == NULL) {
		bucket->predef = true;
		return;
	}

	// Resets
	ctx->debug.scope = func->name->unary_expr.val.strval;
	ctx->debug.ret = false;
	ctx->debug.ret_type = func->type;

	struct Opp_Namespace* temp = ctx->curr_ns;
	ctx->curr_ns = init_namespace(temp, (void*)alloc);
	// func->scope = ctx->curr_ns;

	opp_analize_args(ctx, fn, bucket);

	if (func->body->type != STMT_BLOCK)
		opp_semantics_error(ctx, fn, "Expected block after function '%s'", 
			func->name->unary_expr.val.strval);

	for (unsigned int i = 0; i < func->body->block_stmt.len; i++)
		opp_analize_stmt(ctx, func->body->block_stmt.stmts[i]);

	if (ctx->debug.ret == 0 && bucket->sym_type.decl->t_type != TYPE_VOID)
		opp_warning(ctx, fn, "No return statement found, expected '%s', depth '%d'",
			bucket->sym_type.decl->id, bucket->sym_type.depth);

	ctx->curr_ns = temp;
	ctx->debug.scope = NULL;
}

static void opp_analize_args(struct Opp_Analize* ctx, struct Opp_Node* fn, struct Opp_Bucket* bucket)
{
	for (unsigned int i = 0; i < fn->fn_stmt.len; i++) {
		switch (fn->fn_stmt.args[i].var_stmt.var->type)
		{
			case EUNARY:
			case EELEMENT:
			break;

			default:
				opp_semantics_error(ctx, fn, "Unexpected function argument declaration #%d", i+1);
		}

		if (fn->fn_stmt.args[i].var_stmt.type.decl->t_type == TYPE_STRUCT && 
			fn->fn_stmt.args[i].var_stmt.type.depth == 0)
			opp_semantics_error(ctx, fn, "Cannot have struct object as parameter");

		opp_analize_stmt(ctx, &fn->fn_stmt.args[i]);
	}
}

static unsigned int opp_get_plain_size(struct Opp_Type_Decl* type)
{
	if (type->depth > 0 && type->size == 0)
		return 8;
	
	switch (type->decl->t_type)
	{
		case TYPE_VOID:   return 0;
		case TYPE_I8:     return 1;
		case TYPE_I16:    return 2;
		case TYPE_I32:    return 4;
		case TYPE_I64:    return 8;
		case TYPE_FLOAT:  return 4;
		case TYPE_DOUBLE: return 8;
		case TYPE_STRUCT: return type->decl->l_mem;
		default: break;
	}

	return 0;
}

static void opp_add_to_struct(struct Opp_Analize* ctx, struct Opp_Node* var, unsigned int off)
{
	if (var->var_stmt.var->type != EUNARY)
		opp_semantics_error(ctx, var, "Variables inside struct can only be declared");

	struct Opp_Bucket* bucket = env_get_item(ctx->curr_ns, var->var_stmt.var->unary_expr.val.strval);

	bucket->offset = (int32_t)off;
}

static void opp_analize_struct(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	struct Opp_Type_Entry* t = get_type(&ctx->parser->tree, node->struct_stmt.name);
	node->struct_stmt.ns = init_namespace(NULL, (void*)alloc);
	struct Opp_Namespace* temp = ctx->curr_ns;
	ctx->curr_ns = node->struct_stmt.ns;

	unsigned int allign = 0;
	unsigned int cur_size = 0;
	struct Opp_Node** var = node->struct_stmt.elems;

	// Find largest type
	for (unsigned int i = 0; i < node->struct_stmt.len; i++) {
		opp_analize_var(ctx, node->struct_stmt.elems[i]);
		if (node->struct_stmt.elems[i]->type != STMT_VAR)
			opp_semantics_error(ctx, node, 
				"'%s' struct elements can only be var declarations", node->struct_stmt.name);

		if (node->struct_stmt.elems[i]->var_stmt.type.decl->t_type == TYPE_STRUCT &&
			node->struct_stmt.elems[i]->var_stmt.type.depth == 0 && 
			t == node->struct_stmt.elems[i]->var_stmt.type.decl)
				opp_semantics_error(ctx, node, "Cannot make struct obj of '%s' inside '%s'", t->id, t->id);
		
		unsigned int s = 0;

		if (var[i]->var_stmt.type.decl->t_type == TYPE_STRUCT 
			&& var[i]->var_stmt.type.depth == 0)
			s = var[i]->var_stmt.type.decl->l_mem;
		else 
			s = opp_get_plain_size(&var[i]->var_stmt.type);

		if (s > allign)
			allign = s;
	}

	// Calculate member offsets
	for (unsigned int i = 0; i < node->struct_stmt.len; i++) {
		unsigned int s = opp_get_plain_size(&var[i]->var_stmt.type);

		if (var[i]->var_stmt.type.decl->t_type != TYPE_I8) {
			if (cur_size != 0) {
				while (cur_size % s != 0) 
					cur_size++;
			}
		}

		if (var[i]->var_stmt.type.size > 0)
			s = var[i]->var_stmt.type.size;
		else if (var[i]->var_stmt.type.decl->t_type == TYPE_STRUCT &&
				var[i]->var_stmt.type.depth == 0)
			s = var[i]->var_stmt.type.decl->size;

		opp_add_to_struct(ctx, var[i], cur_size);

		cur_size += s;
	}

	if (node->struct_stmt.len != 0) {
		while (cur_size % allign != 0)
			cur_size++;
	}
	t->l_mem = allign;
	t->size = cur_size;
	ctx->curr_ns = temp;
}

static void opp_analize_stmt(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	switch (node->type)
	{
		case STMT_VAR:
			opp_analize_var(ctx, node);
			break;

		case STMT_RET:
			opp_analize_ret(ctx, node);
			break;

		case EASSIGN:
			opp_analize_assign(ctx, node);
			break;

		case ECALL:
			opp_analize_call(ctx, node);
			break;

		case STMT_IF:
			opp_analize_if(ctx, node);
			break;

		case STMT_WHILE:
			opp_analize_while(ctx, node);
			break;

		case STMT_BLOCK:
			opp_analize_block(ctx, node);
			break;

		case STMT_BREAK:
		case STMT_CASE:
			break; // add these

		default:
			// node->type = IGNORE;
			opp_warning(ctx, node, "Unused expression found");

			opp_analize_expr(ctx, node);
			break;
	}
}

static void opp_analize_ret(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	// Returning value from void
	if ((ctx->debug.ret_type.decl->t_type == TYPE_VOID && ctx->debug.ret_type.depth == 0) 
		&& node->ret_stmt.value != NULL)
		opp_semantics_error(ctx, node, "Attempting to return value from 'void' function");

	// Return without value but expected
	if (ctx->debug.ret_type.decl->t_type != TYPE_VOID && node->ret_stmt.value == NULL) {
		ctx->debug.ret = 1;
		opp_warning(ctx, node, "No return value in function which expects value");
		return;
	}

	if (ctx->debug.ret_type.decl->t_type != TYPE_VOID) {
		Opp_Obj type = opp_analize_expr(ctx, node->ret_stmt.value);

		if (type.depth != ctx->debug.ret_type.depth)
			opp_warning(ctx, node, "Returning different pointer depth");
	}

	ctx->debug.ret = 1;
}

static void opp_analize_var(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	struct Opp_Stmt_Var* var = &node->var_stmt;
	struct Opp_Node* name = NULL;
	bool assign = false;

	unsigned int size = get_type_size(&var->type);
	if (size == 0)
		opp_semantics_error(ctx, node, "Cannot use type 'void' to declare a variable");

	switch (var->var->type)
	{
		case EUNARY: {
			name = var->var;
			break;
		}

		case EELEMENT: {
			name = var->var->elem_expr.name;

			while (name->type != EUNARY) {
				if (name->type != EELEMENT)
					opp_semantics_error(ctx, node, 
						"Unexpected token in array declaration");

				if (name->elem_expr.loc->unary_expr.type != TINTEGER)
					opp_semantics_error(ctx, node,
						"Non constant values not allowed in array declaration");
				size *= name->elem_expr.loc->unary_expr.val.i64val;				
				name = name->elem_expr.name;
			}

			if (var->var->elem_expr.loc->unary_expr.type != TINTEGER)
				opp_semantics_error(ctx, node,
					"Non constant values not allowed in array declaration '%s'", name->unary_expr.val.strval);

			size *= var->var->elem_expr.loc->unary_expr.val.i64val;
			var->type.depth = 1;
			var->type.size = size;
			var->var = name;
			break;
		}

		case EASSIGN: {
			name = var->var->assign_expr.ident;

			if (var->var->assign_expr.ident->type == EELEMENT)
				opp_semantics_error(ctx, node, "Assigning array type variables not allowed");

			assign = true;
			break;
		}

		default: break;
	}

	struct Opp_Bucket* bucket = env_add_item(ctx->curr_ns, name->unary_expr.val.strval);

	if (bucket == NULL)
		opp_semantics_error(ctx, node, "Redeclaration of variable '%s'",
			name->unary_expr.val.strval);

	ctx->debug.res_name = name->unary_expr.val.strval;

	bucket->key = name->unary_expr.val.strval;
	bucket->type = ctx->curr_ns == ctx->ns ? TYPE_GLOBAL : TYPE_LOCAL;
	bucket->sym_type = var->type;
	var->bidx = bucket;

	if (assign)
		opp_analize_assign(ctx, var->var);
}

static void opp_analize_block(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	struct Opp_Namespace* temp = ctx->curr_ns;
	node->block_stmt.ns = init_namespace(temp, (void*)alloc);
	ctx->curr_ns = node->block_stmt.ns;

	for (unsigned int i = 0; i < node->block_stmt.len; i++)
		opp_analize_stmt(ctx, node->block_stmt.stmts[i]);

	ctx->curr_ns = temp;
}

static void opp_analize_if(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	opp_analize_time_eval(ctx, node->if_stmt.cond);

	opp_analize_expr(ctx, node->if_stmt.cond);

	if (node->if_stmt.cond->type != ELOGIC && node->if_stmt.cond->type != EUNARY)
		opp_semantics_error(ctx, node, "If statement requires condition");

	opp_analize_stmt(ctx, node->if_stmt.then);

	if (node->if_stmt.other != NULL)
		opp_analize_stmt(ctx, node->if_stmt.other);
}

static void opp_analize_while(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	opp_analize_time_eval(ctx, node->while_stmt.cond);

	opp_analize_expr(ctx, node->while_stmt.cond);

	if (node->while_stmt.cond->type != ELOGIC && node->while_stmt.cond->type != EUNARY)
		opp_semantics_error(ctx, node, "While statement requires condition");

	opp_analize_stmt(ctx, node->while_stmt.then);
}

static Opp_Obj opp_analize_expr(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	switch (node->type)
	{
		case ECALL:
			return opp_analize_call(ctx, node);

		case EBIN:
			return opp_analize_bin(ctx, node);

		case EUNARY:
			return opp_analize_unary(ctx, node);

		case EDOT:
			return opp_analize_dot(ctx, node);

		case EDEREF:
			return opp_analize_deref(ctx, node);

		case EELEMENT:
			return opp_analize_elem(ctx, node);

		case EADDR:
			return opp_analize_addr(ctx, node);

		case ELOGIC:
			return opp_analize_logic(ctx, node);

		case ESUB:
			return opp_analize_sub(ctx, node);

		case EADJUST:
			return opp_analize_adjust(ctx, node);

		case ESIZEOF:
			return opp_analize_sizeof(ctx, node);

		default:
			opp_semantics_error(ctx, node, "Use of unsupported expr");
			break;
	}
}

static struct Opp_Node* opp_analize_time_eval(struct Opp_Analize* ctx, struct Opp_Node* expr)
{
	#define CLEAR_NODE(node, lhs) { \
		temp.line = node->debug.line;\
		temp.colum = node->debug.colum; \
		memset(node, 0, sizeof(struct Opp_Node)); \
		node->type = EUNARY; \
		node->debug = temp; \
		if (lhs->unary_expr.type == TSTR || lhs->unary_expr.type == TINTEGER) \
			node->unary_expr.type = TINTEGER; \
		else \
			node->unary_expr.type = TFLOAT; \
	}

	#define NUM_OP(op) { \
		if (lhs->unary_expr.type == TINTEGER || lhs->unary_expr.type == TSTR) { \
			if (rhs->unary_expr.type == TFLOAT) \
				expr->unary_expr.val.i64val = lhs->unary_expr.val.i64val op (int64_t)rhs->unary_expr.val.f64val; \
			else \
				expr->unary_expr.val.i64val = lhs->unary_expr.val.i64val op rhs->unary_expr.val.i64val; \
		} \
		else { \
			if (rhs->unary_expr.type == TINTEGER) \
				expr->unary_expr.val.f64val = lhs->unary_expr.val.f64val op (double)rhs->unary_expr.val.i64val; \
			else \
				expr->unary_expr.val.f64val = lhs->unary_expr.val.f64val op rhs->unary_expr.val.f64val; \
		} \
	}

	#define CMP_OP(op) { \
		if (lhs->unary_expr.type == TINTEGER || lhs->unary_expr.type == TSTR) { \
			if (rhs->unary_expr.type == TFLOAT) \
				expr->unary_expr.val.i64val = lhs->unary_expr.val.i64val op (int64_t)rhs->unary_expr.val.f64val; \
			else \
				expr->unary_expr.val.i64val = lhs->unary_expr.val.i64val op rhs->unary_expr.val.i64val; \
		} \
		else { \
			if (rhs->unary_expr.type == TINTEGER) \
				expr->unary_expr.val.i64val = lhs->unary_expr.val.f64val op (double)rhs->unary_expr.val.i64val; \
			else \
				expr->unary_expr.val.i64val = lhs->unary_expr.val.f64val op rhs->unary_expr.val.f64val; \
		} \
	}

	struct Opp_Node* lhs = NULL;
	struct Opp_Node* rhs = NULL;
	struct Opp_Debug temp;

	switch (expr->type)
	{
		case EUNARY: {
			if (expr->unary_expr.type == TIDENT)
				return NULL;
			return expr;
		}

		case EBIN: {
			// TODO: add x * 0 and x* 1 optimizations
			lhs = opp_analize_time_eval(ctx, expr->bin_expr.left);
			rhs = opp_analize_time_eval(ctx, expr->bin_expr.right);
			
			if (lhs == NULL || rhs == NULL) return NULL;

			const int op = expr->bin_expr.tok;
			CLEAR_NODE(expr, lhs);


			switch (op)
			{
				case TADD: NUM_OP(+=); break;
				case TMIN: NUM_OP(-=); break;
				case TMUL: NUM_OP(*=); break;
				case TDIV: NUM_OP(/=); break;
				case TMOD: 
					if (lhs->unary_expr.type != TINTEGER && lhs->unary_expr.type != TSTR)
						opp_semantics_error(ctx, expr, "Mod operation can only be preformed on integers");
					lhs->unary_expr.val.i64val %= rhs->unary_expr.val.i64val;
					break;

				default: break;
			}

			return expr;
		}

		case ELOGIC: {
			lhs = opp_analize_time_eval(ctx, expr->logic_expr.left);
			rhs = opp_analize_time_eval(ctx, expr->logic_expr.right);
			
			if (lhs == NULL || rhs == NULL) return NULL;

			const int op = expr->logic_expr.tok;
			CLEAR_NODE(expr, lhs);
			expr->unary_expr.type = TINTEGER;

			switch (op)
			{
				case TEQEQ:  CMP_OP(==); break;
				case TNOTEQ: CMP_OP(!=); break;
				case TLT:    CMP_OP(<);  break;
				case TLE:    CMP_OP(<=); break;
				case TGT:    CMP_OP(>);  break;
				case TGE:    CMP_OP(>=); break;
				case TOR:    CMP_OP(||); break;
				case TAND:   CMP_OP(&&); break;
				default: break;
			}

			return expr;
		}

		case ESUB: {
			lhs = opp_analize_time_eval(ctx, expr->sub_expr.unary);
			
			if (lhs == NULL) return NULL;
			
			CLEAR_NODE(expr, lhs);
			if (lhs->unary_expr.type == TSTR)
				opp_semantics_error(ctx, expr, "Attempting to negate str '%s'",
					lhs->unary_expr.val.strval);

			if (lhs->unary_expr.type == TINTEGER)
				expr->unary_expr.val.i64val = -lhs->unary_expr.val.i64val;
			else
				expr->unary_expr.val.f64val = -lhs->unary_expr.val.f64val;

			return expr;
		}

		// TODO: Add sizeof here

		default: break;
	}
	return NULL;
}

static Opp_Obj opp_analize_unary(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	Opp_Obj type;

	switch (node->unary_expr.type)
	{
		case TINTEGER:
			type.decl = int_type;
			type.depth = 0;
			type.size = 0;
			type.unsign = 0;
			break;

		case TSTR:
			type.decl = char_type;
			type.depth = 1;
			type.size = 0;
			type.unsign = 0;
			break;
		

		case TFLOAT:
			type.decl = float_type;
			type.depth = 0;
			type.size = 0;
			type.unsign = 0;
			break;
		
		case TIDENT: {
			struct Opp_Bucket* bucket = env_get_item(ctx->curr_ns, node->unary_expr.val.strval);

			if (bucket == NULL)
				opp_semantics_error(ctx, node, "Use of undeclared variable '%s'",
					node->unary_expr.val.strval);

			if (bucket->type == TYPE_LABEL)
				opp_semantics_error(ctx, node, "Invalid use of label '%s'",
					bucket->key);
			
			node->unary_expr.bidx = bucket;
			ctx->debug.res_name = bucket->key;

			return bucket->sym_type;
		}
		default: break;
	}
	return type;
}

static Opp_Obj opp_analize_bin(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	opp_analize_time_eval(ctx, node);

	if (node->type == EUNARY)
		return opp_analize_unary(ctx, node);

	/* ### Swap sides optimization for compiler ### */
	/* Rules
	Dont do on div / mod
	1: 2 + a / 2.3 + a
		Swap immidiates to left
	2: m + test()
		Swap function calls to right

	3: a + b
		No swap
	4: test() + test()
		No swap
	*/
	if (node->bin_expr.tok != TDIV && node->bin_expr.tok != TMOD) {
		struct Opp_Node* temp = NULL;
		if ((node->bin_expr.left->type == EUNARY) &&
			(node->bin_expr.left->unary_expr.type == TINTEGER 
			|| node->bin_expr.left->unary_expr.type == TFLOAT)) 
		{
			// Swap imm to left side
			temp = node->bin_expr.left;
			node->bin_expr.left = node->bin_expr.right;
			node->bin_expr.right = temp;
		}
		else if (node->bin_expr.right->type == ECALL) {
			// Swap func call to left side
			temp = node->bin_expr.left;
			node->bin_expr.left = node->bin_expr.right;
			node->bin_expr.right = temp;
		}
	}

	const char *l_name, *r_name;
	Opp_Obj lhs = opp_analize_expr(ctx, node->bin_expr.left);
	l_name = ctx->debug.res_name;
	Opp_Obj rhs = opp_analize_expr(ctx, node->bin_expr.right);
	r_name = ctx->debug.res_name;

	// Cannot add struct obj of non ptr type
	if ((rhs.decl->t_type == TYPE_STRUCT && rhs.depth == 0) 
		|| 
	   (lhs.decl->t_type == TYPE_STRUCT && lhs.depth == 0))
		opp_semantics_error(ctx, node, "Not allowed to preform arith on struct type");

	// Warn adding two pointer types
	if (rhs.depth > 0 && lhs.depth > 0)
		opp_warning(ctx, node, "Adding two pointers '%s' '%s'", l_name, r_name);

	// Warn array type in arith
	if (rhs.size > 0 || lhs.size > 0)
		opp_warning(ctx, node, "Adding two arrays causes undefined behavior");

	// Error adding to pointer a floating point
	if ((rhs.depth > 0 || lhs.depth > 0) 
		&& 
		((rhs.decl->t_type == TYPE_FLOAT || rhs.decl->t_type == TYPE_DOUBLE)
			|| (lhs.decl->t_type == TYPE_FLOAT || lhs.decl->t_type == TYPE_DOUBLE)))
		opp_semantics_error(ctx, node, "Adding a floating point type to pointer");

	/* ### Pointer size optimization ### */
	/* Rules
	Only applies to add and sub
	double* a;
	1: a + 2
		Make 2 actually 2 * sizeof(double)
	
	int** b;
	2: b + 3
		Make 3 actually 3 * sizeof(int**)
	*/
	if (node->bin_expr.tok == TADD || node->bin_expr.tok == TMIN) {
		if (lhs.depth > 0 && (node->bin_expr.right->type == EUNARY 
			&& node->bin_expr.right->unary_expr.type == TINTEGER))
		{
			lhs.depth--;
			unsigned int size = get_type_size(&lhs);
			node->bin_expr.right->unary_expr.val.i64val *= size;
			lhs.depth++;
		}
	}

	return lhs;
}

static Opp_Obj opp_analize_logic(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	opp_analize_time_eval(ctx, node);

	if (node->type == EUNARY) return opp_analize_unary(ctx, node);

	Opp_Obj lhs = opp_analize_expr(ctx, node->logic_expr.left);
	Opp_Obj rhs = opp_analize_expr(ctx, node->logic_expr.right);

	// Warning different pointer depths compare
	if (lhs.depth != rhs.depth)
		opp_warning(ctx, node, "Preforming compare operation mixing with ptr '%s' '%s'",
			lhs.decl->id, rhs.decl->id);

	// Warn different type pointer compare
	if ((lhs.depth > 0 && rhs.depth > 0) && lhs.decl->t_type != rhs.decl->t_type)
		opp_warning(ctx, node, "Preforming logic on different point types");

	// Mixing struct in compare error
	if ((rhs.decl->t_type == TYPE_STRUCT && rhs.depth == 0) 
		|| 
		(lhs.decl->t_type == TYPE_STRUCT && lhs.depth == 0))
		opp_semantics_error(ctx, node, "Not allowed to mix struct type in logic operation");

	return lhs;
}

static Opp_Obj opp_analize_assign(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	if ((node->assign_expr.ident->type != EUNARY 
		|| node->assign_expr.ident->unary_expr.type != TIDENT)
		&& (node->assign_expr.ident->type != EDEREF) 
		&& (node->assign_expr.ident->type != EELEMENT)
		&& (node->assign_expr.ident->type != EDOT))
		opp_semantics_error(ctx, node, "Unexpected expression on lhs of '='");

	Opp_Obj lhs = opp_analize_expr(ctx, node->assign_expr.ident);

	const char* name = ctx->debug.res_name;

	// Global assign error
	if (ctx->curr_ns == ctx->ns)
		opp_semantics_error(ctx, node, "Global assigments are not permited, assiment of '%s'", name);

	// Cannot assign array error
	if (lhs.size > 0)
		opp_semantics_error(ctx, node, "Attempting to assign a value to array '%s'", ctx->debug.res_name);

	Opp_Obj rhs = opp_analize_expr(ctx, node->assign_expr.val);

	// Cannot assign non pointer to array type
	if (rhs.size > 0 && lhs.depth == 0) 
		opp_semantics_error(ctx, node, "Attempting to assign an array type to var '%s'", name);

	// Different depths warning	
	if (rhs.depth != lhs.depth)
		opp_warning(ctx, node, "Assigning var '%s' to incompatible pointer depth", name);

	// Cannot assign non pointer to obj and vise versa
	if ((rhs.decl->t_type == TYPE_STRUCT && rhs.depth == 0)
		|| 
		(lhs.decl->t_type == TYPE_STRUCT && lhs.depth == 0))
		opp_semantics_error(ctx, node, "Assigning var '%s' to a incompatible struct type", name);

	return lhs;
}

static Opp_Obj opp_analize_call(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	Opp_Obj ret_type;

	ret_type = opp_analize_expr(ctx, node->call_expr.callee);

	// Immidiate func call
	if (node->call_expr.callee->type == EUNARY) {
		if (node->call_expr.callee->unary_expr.type != TIDENT)
			opp_semantics_error(ctx, node, "Expected identifier before '()' for a call expr");

		struct Opp_Bucket* bucket = env_get_item(ctx->curr_ns, 
			node->call_expr.callee->unary_expr.val.strval);

		if (bucket == NULL)
			opp_semantics_error(ctx, node, "Attempting to call undefined symbol'%s'",
				node->call_expr.callee->unary_expr.val.strval);

		node->call_expr.callee->unary_expr.bidx = bucket;

		char* fn_name = bucket->key;

		if (bucket->type == TYPE_FUNC || bucket->type == TYPE_EXTERN) {
			if (bucket->args->len != node->call_expr.args->length)
				opp_semantics_error(ctx, node, 
					"Invalid amount of args provided in function call to '%s' (expected %d)", 
					fn_name, bucket->args->len);
		}
		else if (bucket->sym_type.depth == 0)
			opp_semantics_error(ctx, node, "Calling a invalid callee '%s'", fn_name);

		for (unsigned int i = 0; i < node->call_expr.args->length; i++) 
		{
			ret_type = opp_analize_expr(ctx, node->call_expr.args->list[i]);

			if (bucket->type == TYPE_FUNC || bucket->type == TYPE_EXTERN)
				if (ret_type.depth != bucket->args->args[i].var_stmt.type.depth) 
					opp_warning(ctx, node, 
						"Invalid pointer argument provided to function call '%s'", fn_name);
		}
		return bucket->sym_type;
	}
	if (node->call_expr.args->length > 0)
		printf("ADD FN POINTER ARGS\n");

	return ret_type;
}

static Opp_Obj opp_analize_dot(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	Opp_Obj lhs = opp_analize_expr(ctx, node->dot_expr.left);

	// Error if attempint dot access on non struct
	if (lhs.decl->t_type != TYPE_STRUCT || lhs.depth > 0)
		opp_semantics_error(ctx, node, "Attempting to access element on a non object type '%s'",
			ctx->debug.res_name);

	const char* left = ctx->debug.res_name;

	// Error if no identifier memeber
	if (node->dot_expr.right->type != EUNARY || node->dot_expr.right->unary_expr.type != TIDENT)
		opp_semantics_error(ctx, node, "Expected element on right side of '%s' dot expr", left);

	// TODO: Rethink if better strategy
	// Switch to struct namespace
	struct Opp_Namespace* temp = ctx->curr_ns;
	ctx->curr_ns = lhs.decl->s_elems->ns;

	Opp_Obj rhs = opp_analize_expr(ctx, node->dot_expr.right);
	
	ctx->curr_ns = temp;

	return rhs;
}

static Opp_Obj opp_analize_deref(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	// Error if lhs of deref is invalid
	if ((node->deref_expr.deref->type != EUNARY || node->deref_expr.deref->unary_expr.type != TIDENT)
		&& (node->deref_expr.deref->type != EBIN) && (node->deref_expr.deref->type != EADDR)
		&& (node->deref_expr.deref->type != EDEREF) && (node->deref_expr.deref->type != EDOT
		&& (node->deref_expr.deref->type != EELEMENT)))
			opp_semantics_error(ctx, node, "Attempt to dereference invalid expression");

	Opp_Obj lhs = opp_analize_expr(ctx, node->deref_expr.deref);

	if (lhs.depth == 0)
		opp_semantics_error(ctx, node, 
			"Attempting to dereference a non pointer type '%s'", ctx->debug.res_name);

	lhs.depth--;

	return lhs;
}

static Opp_Obj opp_analize_sub(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	opp_analize_time_eval(ctx, node);

	if (node->type == EUNARY)
		return opp_analize_unary(ctx, node);

	assert(false);
}

static Opp_Obj opp_analize_elem(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	Opp_Obj lhs = opp_analize_expr(ctx, node->elem_expr.name);

	if (node->elem_expr.name->type == EELEMENT)
		return lhs;

	Opp_Obj rhs = opp_analize_expr(ctx, node->elem_expr.loc);

	if (lhs.depth == 0 && rhs.depth == 0)
		opp_semantics_error(ctx, node, "Attemping to get element of non pointer / array type '%s'",
			ctx->debug.res_name);

	lhs.size = 0;
	if (lhs.depth > 0)
		lhs.depth--;

	return lhs;
}

static Opp_Obj opp_analize_addr(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	if (node->addr_expr.addr->type == EUNARY 
		&& (node->addr_expr.addr->unary_expr.type == TINTEGER 
			|| node->addr_expr.addr->unary_expr.type == TFLOAT))
		opp_semantics_error(ctx, node, "Got unexpected rhs value in '&'");

	Opp_Obj lhs = opp_analize_expr(ctx, node->addr_expr.addr);
	lhs.depth++;

	return lhs;
}

static Opp_Obj opp_analize_adjust(struct Opp_Analize* ctx, struct Opp_Node* node)
{

}

static Opp_Obj opp_analize_sizeof(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	Opp_Obj val = opp_analize_expr(ctx, node->sizeof_expr.expr);

	struct Opp_Debug temp = node->debug;
	memset(node, 0, sizeof(struct Opp_Node));
	node->type = EUNARY;
	node->debug = temp;
	node->unary_expr.type = TINTEGER;
	node->unary_expr.val.i64val = get_type_size(&val);

	val.decl = int_type;
	val.depth = 0;
	val.size = 0;
	val.unsign = 0;

	return val;
}