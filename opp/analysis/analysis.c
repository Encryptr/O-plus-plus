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

static void opp_zero_node(struct Opp_Node* node);

static void opp_analize_global(struct Opp_Analize* ctx, struct Opp_Node* node);
static void opp_analize_func(struct Opp_Analize* ctx, struct Opp_Node* fn);
static void opp_analize_args(struct Opp_Analize* ctx, struct Opp_Node* fn, struct Opp_Bucket* bucket);

static void opp_analize_stmt(struct Opp_Analize* ctx, struct Opp_Node* node);
static void opp_analize_var(struct Opp_Analize* ctx, struct Opp_Node* node);

struct Opp_Analize* opp_init_analize(struct Opp_Parser* parser)
{
	struct Opp_Analize* ctx = (struct Opp_Analize*)malloc(sizeof(struct Opp_Analize));

	if (ctx == NULL)
		goto err;

	ctx->parser = parser;
	ctx->debug.scope = NULL;
	ctx->debug.ret = 0;
	ctx->debug.loc_var = 0;
	ctx->debug.loc_global = 0;

	// Namespace
	ctx->ns = init_namespace(NULL, (void*)alloc);
	ctx->curr_ns = ctx->ns;

	// // Ops
	// ctx->ops.tree = (struct Analyzer_Node**)
	// 	malloc(sizeof(struct Analyzer_Node*)*DEFAULT_ANALYZER_OPS);

	// if (ctx->ops.tree == NULL)
	// 	goto err;

	// ctx->ops.allocated = DEFAULT_ANALYZER_OPS;
	// ctx->ops.idx = 0;

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

static void opp_zero_node(struct Opp_Node* node)
{
	// memset()
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

		// case STMT_IMPORT:
		// 	opp_import_module(opp, opp->parser->statments[stmt]);
		// 	break;

		// case STMT_VAR:
		// 	opp_compile_var(opp, opp->parser->statments[stmt]);
		// 	break;

		// case TEXTERN:
		// 	return opp_parse_extern(parser);

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
	bool global = ctx->curr_ns == ctx->ns;

	// for (unsigned int i = 0; i < var->);
}