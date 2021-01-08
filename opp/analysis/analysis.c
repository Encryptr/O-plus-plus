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

static void opp_set_sym_idx(struct Opp_Node* node, unsigned int idx);

// Globals
static void opp_analize_global(struct Opp_Analize* ctx, struct Opp_Node* node);
static void opp_analize_func(struct Opp_Analize* ctx, struct Opp_Node* fn);
static void opp_analize_args(struct Opp_Analize* ctx, struct Opp_Node* fn, struct Opp_Bucket* bucket);

// Stmt
static void opp_analize_stmt(struct Opp_Analize* ctx, struct Opp_Node* node);
static void opp_analize_var(struct Opp_Analize* ctx, struct Opp_Node* node);

// Expr
static struct Opp_Type_Decl opp_analize_expr(struct Opp_Analize* ctx, struct Opp_Node* node);
static struct Opp_Type_Decl opp_analize_unary(struct Opp_Analize* ctx, struct Opp_Node* node);

struct Opp_Analize* opp_init_analize(struct Opp_Parser* parser, struct Opp_Options* opts)
{
	struct Opp_Analize* ctx = (struct Opp_Analize*)malloc(sizeof(struct Opp_Analize));

	if (ctx == NULL)
		goto err;

	ctx->opts = opts;

	ctx->parser = parser;
	ctx->debug.scope = NULL;
	ctx->debug.ret = 0;
	ctx->debug.loc_var = 0;
	ctx->debug.loc_global = 0;

	// Namespace
	ctx->ns = init_namespace(NULL, (void*)alloc);
	ctx->curr_ns = ctx->ns;

	return ctx;

err:
	INTERNAL_ERROR("Malloc Fail");
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

static void opp_set_sym_idx(struct Opp_Node* node, unsigned int idx)
{
	if (node->type != EUNARY)
		return;

	node->unary_expr.type = TINTEGER;
	node->unary_expr.val.i64val = (int64_t)idx;
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

		// case STMT_EXTERN:
		// 	opp_compile_extern(opp, opp->parser->statments[stmt]);
		// 	break;

		case STMT_VAR:
			opp_analize_var(ctx, node);
			break;

		default: break;
	}
}

static void opp_analize_func(struct Opp_Analize* ctx, struct Opp_Node* fn)
{
	struct Opp_Stmt_Func* func = &fn->fn_stmt;

	struct Opp_Bucket* bucket = env_add_item(ctx->curr_ns, func->name->unary_expr.val.strval);

	if (bucket == NULL)
		opp_semantics_error(ctx, fn, "Redeclaration of function '%s'", func->name->unary_expr.val.strval);

	bucket->sym_idx = ctx->debug.loc_global++;

	// Resets
	ctx->debug.scope = func->name->unary_expr.val.strval;
	ctx->debug.ret = 0;
	ctx->debug.ret_type = func->type;
	ctx->debug.loc_var = 0;

	struct Opp_Namespace* temp = ctx->curr_ns;
	ctx->curr_ns = init_namespace(temp, (void*)alloc);

	opp_analize_args(ctx, fn, bucket);

	if (func->body->type != STMT_BLOCK)
		opp_semantics_error(ctx, fn, "Expected block in function '%s'", 
			func->name->unary_expr.val.strval);

	for (unsigned int i = 0; i < func->body->block_stmt.len; i++)
		opp_analize_stmt(ctx, func->body->block_stmt.stmts[i]);

	ctx->curr_ns = temp;
	ctx->debug.scope = NULL;
}

static void opp_analize_args(struct Opp_Analize* ctx, struct Opp_Node* fn, struct Opp_Bucket* bucket)
{
	// for (unsigned int i = 0; i < fn->fn_stmt.args->length; i++) {

	// }
}

static void opp_analize_stmt(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	switch (node->type)
	{
		case STMT_VAR:
			opp_analize_var(ctx, node);
			break;
	}
}

static void opp_analize_var(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	struct Opp_Stmt_Var* var = &node->var_stmt;
	struct Opp_Node* name = NULL;
	struct Opp_Type_Decl rhs;

	var->global = ctx->ns == ctx->curr_ns;

	switch (var->var->type)
	{
		case EUNARY: {
			name = var->var;
			break;
		}

		case EELEMENT: {
			break;
		}

		case EASSIGN: { // add rule for global var assign
			name = var->var->assign_expr.ident;
			rhs = opp_analize_expr(ctx, var->var->assign_expr.val);

			if (rhs.size > 0 && var->type.depth == 0) // check this
				opp_semantics_error(ctx, node, "Invalid array assigment in assigment of '%s'",
					name->unary_expr.val.strval);

			if (rhs.depth != var->type.depth)
				opp_warning(ctx, node, "Assigning var '%s' to incompatible pointer depth of type '%s'",
					name->unary_expr.val.strval, rhs.decl->id);

			if ((rhs.decl->t_type == TYPE_STRUCT || var->type.decl->t_type == TYPE_STRUCT)
				&& rhs.decl->t_type != var->type.decl->t_type)
				opp_semantics_error(ctx, node, "Assigning var '%s' to a incompatile type struct",
					name->unary_expr.val.strval);
			break;
		}
	}

	struct Opp_Bucket* bucket = env_add_item(ctx->curr_ns, name->unary_expr.val.strval);

	if (bucket == NULL)
		opp_semantics_error(ctx, node, "Redeclaration of variable '%s'",
			name->unary_expr.val.strval);

	bucket->key = name->unary_expr.val.strval;
	bucket->type = ctx->curr_ns == ctx->ns ? TYPE_GLOBAL : TYPE_LOCAL;
	bucket->t_type = var->type;
	bucket->sym_idx = ctx->debug.loc_var++;
}


static struct Opp_Type_Decl opp_analize_expr(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	// do const expr eval
	switch (node->type)
	{
		// case ECALL:
		// 	opp_compile_call(opp, expr);
		// 	break;

		// case EASSIGN:
		// 	type = opp_compile_assign(opp, expr);
		// 	break;

		// case EBIN:
		// 	type = opp_compile_bin(opp, expr);
		// 	break;

		case EUNARY:
			return opp_analize_unary(ctx, node);

		// case EDEREF:
		// 	type = opp_compile_deref(opp, expr);
		// 	break;

		// case EADDR:
		// 	type = opp_compile_addr(opp, expr);
		// 	break;

		// case ELOGIC:
		// 	type = opp_compile_logic(opp, expr);
		// 	break;

		// case ESUB:
		// 	type = opp_compile_sub(opp, expr);
		// 	break;

		default: break;
	}
}

static struct Opp_Type_Decl opp_analize_unary(struct Opp_Analize* ctx, struct Opp_Node* node)
{
	struct Opp_Type_Decl type;

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

			node->unary_expr.global = 0;

			if (bucket->type == TYPE_GLOBAL || bucket->type == TYPE_FUNC)
				node->unary_expr.global = 1;

			opp_set_sym_idx(node, bucket->sym_idx);

			return bucket->t_type;
		}
		default: break;
	}
	return type;
}