#include <stdio.h>
#include "../opp.h"
#include "analizer.h"
#include "../parser/parser.h"
#include "../parser/types.h"
#include "../ast/ast.h"
#include "../util/util.h"
#include "../platform.h"

static void opp_analize_global(struct Opp_Analyzer* opp, struct Opp_Stmt* node);
static void opp_analize_func(struct Opp_Analyzer* opp, struct Opp_Stmt* node);
static void opp_analize_decl(struct Opp_Analyzer* opp, struct Opp_Stmt* node);

static void opp_semantic_error(struct Opp_Analyzer* opp, struct Opp_Debug* info, 
								const char* str, ...)
{
	va_list ap;
	va_start(ap, str);
	
	printf("%s:%u:%u ", opp->parser->lex->io.fname, info->line, info->colum);
	#ifdef UNX 
	printf(CL_RED);
	#endif

	if (!opp->debug.fn_name)
		printf("semantics error:\n\t");
	else
		printf("semantics error: in func '%s'\n\t", opp->debug.fn_name);
	printf(CL_RESET); 
	vfprintf(stdout, str, ap);
	fprintf(stdout, "\n");
	va_end(ap);

	longjmp(global_state.error_buf, OPP_ERROR);
}

void opp_start_analyzer(struct Opp_Parser* parser)
{
	struct Opp_Analyzer opp = {0};
	opp.parser = parser;
	opp.global = opp_create_map(GLOABL_MAP_SIZE, NULL);
	opp.scope = opp.global;
	opp.debug.linkage = SCOPE_GLOBAL;
	opp.debug.fn_name = NULL;

	for (unsigned int i = 0; i < parser->nstmts; i++)
		opp_analize_global(&opp, parser->statements[i]);
}

static void opp_analize_global(struct Opp_Analyzer* opp, struct Opp_Stmt* node)
{
	switch (node->type) 
	{
		case STMT_FUNC:
			opp_analize_func(opp, node);
			break;

		case STMT_DECL:
			opp_analize_decl(opp, node);
			break;

		default: 
			break;
	}
}

static void opp_analize_func(struct Opp_Analyzer* opp, struct Opp_Stmt* node)
{
	struct Opp_Stmt_Func* func = &node->stmt.func;
	struct Opp_Bucket* bucket = opp_get_bucket(opp->scope, func->name);

	opp->debug.fn_name = func->name;

	if (!bucket) {
		// define it
	}
	else {
		// type check 
	}

	if (func->type->next->type == TYPE_FUNC)
		opp_semantic_error(opp, &node->debug, "Function cannot return function type");

	// check function args (func to pointer, array to pointer)

	// analize body stmts

	opp.debug.fn_name = NULL;
}

static void opp_analize_decl(struct Opp_Analyzer* opp, struct Opp_Stmt* node)
{

}