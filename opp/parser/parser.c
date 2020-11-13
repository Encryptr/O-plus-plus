/** @file parser.c
 * 
 * @brief Opp Parser 
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

#include "parser.h"

#define DEBUG_PARSE

struct Opp_Parser* opp_parser_init(struct Opp_Scan* s)
{
	struct Opp_Parser* parser = (struct Opp_Parser*)
		malloc(sizeof(struct Opp_Parser));

	parser->lex = s;
	parser->nstmts = 0;
	parser->statments = calloc(64, sizeof(*parser->statments));
	parser->stmt_size = 64;

	if (parser->statments == NULL)
		INTERNAL_ERROR("Malloc Fail");

	return parser;
}

void opp_parser_begin(struct Opp_Parser* parser)
{
	if (parser == NULL)
		return;

	for (;;) {
		opp_next(parser->lex);
		if (parser->lex->t.id == FEND)
			break;

		struct Opp_Node* stmt = opp_parse_global_def(parser);

		if (parser->nstmts == parser->stmt_size) {
			parser->statments = (struct Opp_Node**)
				realloc(parser->statments, (64+parser->stmt_size) * sizeof(struct Opp_Node*));

			if (parser->statments == NULL)
				INTERNAL_ERROR("Malloc Fail");

			parser->stmt_size += 64;
		}

		parser->statments[parser->nstmts] = stmt;
		parser->nstmts++;
	}
}

static struct Opp_Node* opp_new_node(struct Opp_Parser* parser, enum Opp_Node_Type type)
{
	struct Opp_Node* node = (struct Opp_Node*)
		malloc(sizeof(struct Opp_Node));

	if (node == NULL)
		INTERNAL_ERROR("Malloc fail");

	node->type = type;
	node->debug.line = parser->lex->line;
	node->debug.colum = parser->lex->colum;

	return node;
}

static void opp_expect_error(struct Opp_Parser* parser, char sym)
{
	if (*parser->lex->src >= 33 && *parser->lex->src <= 126)
		opp_error(parser->lex, "Expected matching '%c' but found '%c'",
			sym, *parser->lex->src);
	else {
		char* t = parser->lex->src;
		while (*t <= 32) t--;
		opp_error(parser->lex, "Expected matching '%c' near \"%c\"", sym, *t);
	}
}

static void opp_debug_parser(struct Opp_Node* node)
{
	switch (node->type)
	{
		case EASSIGN:
			printf("Assign type: %d\n", node->assign_expr.op);
			printf("Left: %s\n", node->assign_expr.ident->unary_expr.val.strval);
			printf("Value: %ld\n", node->assign_expr.val->unary_expr.val.i64val);
			break;

	}
}

static struct Opp_Node* opp_parse_global_def(struct Opp_Parser* parser)
{
	switch (parser->lex->t.id)
	{
		// Structs / Extern etc...
		case TIDENT: 
			return opp_parse_func(parser);

		case TEXTERN:
			return opp_parse_extern(parser);

		case TIMPORT:
			return opp_parse_import(parser);

		case TAUTO:
			return opp_parse_var_decl(parser);

		default:
			opp_error(parser->lex, 
				"Error unexpected global declaration '%s'", parser->lex->t.buffer.buf);
	}

	return NULL;
}

static struct Opp_Node* opp_parse_func(struct Opp_Parser* parser)
{
	struct Opp_Node* func_node = opp_new_node(parser, STMT_FUNC);

	func_node->fn_stmt.name = opp_parse_unary(parser);
	opp_next(parser->lex);

	if (parser->lex->t.id != TOPENP)
		opp_error(parser->lex, "Expected '(' after function identifier '%s'",
			func_node->fn_stmt.name->unary_expr.val.strval);

	func_node->fn_stmt.args = opp_parse_args(parser);
	opp_next(parser->lex);
	func_node->fn_stmt.body = opp_parse_statement(parser);

	return func_node;
}

#define DEFAULT_LIST_SIZE 8

// ADD BLOCKER
static struct Opp_List* opp_parse_args(struct Opp_Parser* parser)
{
	struct Opp_List* args = (struct Opp_List*)malloc(sizeof(struct Opp_List));
	args->list = (struct Opp_Node**)malloc(sizeof(struct Opp_Node*)*DEFAULT_LIST_SIZE);

	int i = 0;
	do {
		opp_next(parser->lex);
		if (parser->lex->t.id == TCLOSEP 
			|| parser->lex->t.id == FEND)
			break;

		args->list[i] = opp_parse_allign(parser);
		i++;

	} while (parser->lex->t.id == TCOMMA);

	if (parser->lex->t.id != TCLOSEP)
		opp_expect_error(parser, ')');

	args->length = i;

	return args;
}

static struct Opp_Node* opp_parse_statement(struct Opp_Parser* parser)
{
	switch (parser->lex->t.id)
	{
		case TOPENC:
			return opp_parse_block(parser);

		case TAUTO:
			return opp_parse_var_decl(parser);

		case TRET:
			return opp_parse_return(parser);

		case TWHILE:
			return opp_parse_while(parser);

		case TGOTO:
			return opp_parse_goto(parser);

		case TIF:
			return opp_parse_if(parser);

		case TIDENT: 
			if (opp_parse_peak(parser, ':'))
				return opp_parse_label(parser);

		default:
			return opp_parse_expr(parser);
	}

	return NULL;
}

static struct Opp_Node* opp_parse_block(struct Opp_Parser* parser)
{
	int block_init_size = 64;
	struct Opp_Node* block = opp_new_node(parser, STMT_BLOCK);
	block->block_stmt.stmts = calloc(block_init_size, sizeof(struct Opp_Node*));

	if (block->block_stmt.stmts == NULL)
		INTERNAL_ERROR("Malloc fail");

	opp_next(parser->lex);
	int index = 0;

	while (parser->lex->t.id != TCLOSEC && parser->lex->t.id != FEND) {
		if (index == block_init_size)
		{
			block->block_stmt.stmts = (struct Opp_Node**)realloc(block->block_stmt.stmts, 
					(64+block_init_size) * sizeof(struct Opp_Node*));
			if (block == NULL)
				INTERNAL_ERROR("Malloc Fail");
			block_init_size += 64;
		}
		block->block_stmt.stmts[index] = opp_parse_statement(parser);
		index++;

		opp_next(parser->lex);
	}

	block->block_stmt.stmts[index] = NULL;
	block->block_stmt.len = index;

	if (parser->lex->t.id != TCLOSEC || parser->lex->t.id == FEND)
		opp_expect_error(parser, '}');

	return block;
}

static struct Opp_Node* opp_parse_label(struct Opp_Parser* parser)
{
	struct Opp_Node* label = opp_new_node(parser, STMT_LABEL);

	label->label_stmt.name = opp_parse_unary(parser);
	opp_next(parser->lex);

	return label;
}

static struct Opp_Node* opp_parse_var_decl(struct Opp_Parser* parser)
{
	struct Opp_Node* var = opp_new_node(parser, STMT_VAR);

	var->var_stmt.vars = opp_parse_comma(parser);

	return var;
}

static struct Opp_Node* opp_parse_return(struct Opp_Parser* parser)
{
	struct Opp_Node* ret = opp_new_node(parser, STMT_RET);

	opp_next(parser->lex);
	if (parser->lex->t.id == TSEMICOLON) 
		ret->ret_stmt.value = NULL;
	else
		ret->ret_stmt.value = opp_parse_expr(parser);

	return ret;
}

static struct Opp_Node* opp_parse_while(struct Opp_Parser* parser)
{
	struct Opp_Node* loop = opp_new_node(parser, STMT_WHILE);

	opp_next(parser->lex);

	if (parser->lex->t.id != TOPENP)
		opp_error(parser->lex, "Expected '(' after while");

	loop->while_stmt.cond = opp_parse_allign(parser);
	loop->while_stmt.then = opp_parse_statement(parser);

	return loop;
}

static struct Opp_Node* opp_parse_extern(struct Opp_Parser* parser)
{
	struct Opp_Node* extrn = opp_new_node(parser, STMT_EXTERN);

	opp_next(parser->lex);

	extrn->extrn_stmt.func_decl = opp_parse_unary(parser);
	opp_next(parser->lex);
	extrn->extrn_stmt.args = opp_parse_args(parser);
	opp_next(parser->lex);

	if (parser->lex->t.id != TSEMICOLON)
		opp_error(parser->lex, "Expected ';' after extern declaration");

	return extrn;
}

static struct Opp_Node* opp_parse_goto(struct Opp_Parser* parser)
{
	struct Opp_Node* jmp = opp_new_node(parser, STMT_GOTO);

	opp_next(parser->lex);

	if (parser->lex->t.id != TIDENT)
		opp_error(parser->lex, "Expected label after goto");

	jmp->goto_stmt.name = opp_parse_unary(parser);


	opp_next(parser->lex);

	if (parser->lex->t.id != TSEMICOLON)
		opp_expect_error(parser, ';');

	return jmp;
}

static struct Opp_Node* opp_parse_import(struct Opp_Parser* parser)
{
	struct Opp_Node* import = opp_new_node(parser, STMT_IMPORT);

	opp_next(parser->lex);

	import->import_stmt.ident = opp_parse_unary(parser);

	opp_next(parser->lex);

	if (parser->lex->t.id != TSEMICOLON)
		opp_error(parser->lex, "Expected ';' after import statement");

	return import;
}

static struct Opp_Node* opp_parse_if(struct Opp_Parser* parser)
{
	struct Opp_Node* ifstmt = opp_new_node(parser, STMT_IF);

	opp_next(parser->lex);

	if (parser->lex->t.id != TOPENP)
		opp_error(parser->lex, "Expected '(' after if");

	opp_next(parser->lex);

	ifstmt->if_stmt.cond = opp_parse_allign(parser);

	if (parser->lex->t.id != TCLOSEP)
		opp_error(parser->lex, "Expected ')' after if condition");

	opp_next(parser->lex);

	ifstmt->if_stmt.then = opp_parse_statement(parser);

	// use opp_lex_peek

	return ifstmt;
}

static struct Opp_Node* opp_parse_expr(struct Opp_Parser* parser)
{
	struct Opp_Node* new_expr = opp_parse_allign(parser);

	if (parser->lex->t.id != TSEMICOLON) {
		opp_expect_error(parser, ';');
	}

	return new_expr;
}

// ADD BLOCKER
static struct Opp_List* opp_parse_comma(struct Opp_Parser* parser)
{
	int i = 0;
	struct Opp_List* list = (struct Opp_List*)malloc(sizeof(struct Opp_List));

	if (list == NULL)
		INTERNAL_ERROR("Malloc fail");

	list->list = (struct Opp_Node**)malloc(sizeof(struct Opp_Node*)*DEFAULT_LIST_SIZE);

	if (list->list == NULL)
		INTERNAL_ERROR("Malloc fail");

	do {
		opp_next(parser->lex);
		list->list[i] = opp_parse_allign(parser);
		// opp_next(parser->lex);
		i++;
	} while (parser->lex->t.id == TCOMMA);

	list->length = i;
	if (parser->lex->t.id != TSEMICOLON)
		opp_expect_error(parser, ';');

	return list;
}

static bool opp_parse_peak(struct Opp_Parser* parser, char ch)
{
	char* temp = parser->lex->src;
	if (*temp == ch)
		return true;
	return false;
}

static struct Opp_Node* opp_parse_allign(struct Opp_Parser* parser)
{
	struct Opp_Node* left = opp_parse_or(parser);
	struct Opp_Node* right = NULL;
	struct Opp_Node* assign = NULL;

	int operator = parser->lex->t.id;

	while (operator == TEQ)
	{
		opp_next(parser->lex);
		right = opp_parse_or(parser);

		assign = opp_new_node(parser, EASSIGN);
		assign->assign_expr.ident = left;
		assign->assign_expr.val = right;
		assign->assign_expr.op = operator;

		left = assign;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Node* opp_parse_or(struct Opp_Parser* parser)
{
	struct Opp_Node* left = opp_parse_and(parser);
	struct Opp_Node* right = NULL;
	struct Opp_Node* or = NULL;

	int operator = parser->lex->t.id;

	while (operator == TOR) 
	{
		opp_next(parser->lex);
		right = opp_parse_and(parser);

		or = opp_new_node(parser, ELOGIC);
		or->logic_expr.tok = TOR;
		or->logic_expr.left = left;
		or->logic_expr.right = right;

		left = or;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Node* opp_parse_and(struct Opp_Parser* parser)
{
	struct Opp_Node* left = opp_parse_relation(parser);
	struct Opp_Node* right = NULL;
	struct Opp_Node* and = NULL;

	int operator = parser->lex->t.id;

	while (operator == TAND) 
	{
		opp_next(parser->lex);
		right = opp_parse_relation(parser);

		and = opp_new_node(parser, ELOGIC);
		and->logic_expr.tok = TAND;
		and->logic_expr.left = left;
		and->logic_expr.right = right;

		left = and;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Node* opp_parse_relation(struct Opp_Parser* parser)
{
	struct Opp_Node* left = opp_parse_comparison(parser);
	struct Opp_Node* right = NULL;
	struct Opp_Node* relation = NULL;

	int operator = parser->lex->t.id;

	while (operator == TEQEQ || operator == TNOTEQ)
	{
		opp_next(parser->lex);
		right = opp_parse_comparison(parser);

		relation = opp_new_node(parser, ELOGIC);
		relation->logic_expr.tok = operator;
		relation->logic_expr.left = left;
		relation->logic_expr.right = right;

		left = relation;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Node* opp_parse_comparison(struct Opp_Parser* parser)
{
	struct Opp_Node* left = opp_parse_order1(parser);
	struct Opp_Node* right = NULL;
	struct Opp_Node* comp = NULL;

	int operator = parser->lex->t.id;

	while (operator == TGT || operator == TLT ||
			operator == TGE || operator == TLE)
	{
		opp_next(parser->lex);
		right = opp_parse_order1(parser);

		comp = opp_new_node(parser, ELOGIC);
		comp->logic_expr.tok = operator;
		comp->logic_expr.left = left;
		comp->logic_expr.right = right;

		left = comp;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Node* opp_parse_order1(struct Opp_Parser* parser)
{
	struct Opp_Node* left = opp_parse_order2(parser);
	struct Opp_Node* right = NULL;
	struct Opp_Node* result = NULL;

	int operator = parser->lex->t.id;

	while (operator == TADD || operator == TMIN)
	{
		opp_next(parser->lex);
		right = opp_parse_order2(parser);

		result = opp_new_node(parser, EBIN);
		result->bin_expr.tok = operator;
		result->bin_expr.left = left;
		result->bin_expr.right = right;

		left = result;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Node* opp_parse_order2(struct Opp_Parser* parser)
{
	struct Opp_Node* left = opp_parse_before(parser);
	struct Opp_Node* right = NULL;
	struct Opp_Node* result = NULL;

	int operator = parser->lex->t.id;

	while (operator == TDIV || operator == TMUL || 
		operator == TMOD)
	{
		opp_next(parser->lex);
		right = opp_parse_before(parser);

		result = opp_new_node(parser, EBIN);
		result->bin_expr.tok = operator;
		result->bin_expr.left = left;
		result->bin_expr.right = right;

		left = result;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Node* opp_parse_before(struct Opp_Parser* parser)
{
	struct Opp_Node* value = NULL;
	
	if (parser->lex->t.id == TMIN)
	{
		value = opp_new_node(parser, ESUB);
		opp_next(parser->lex);
		value->sub_expr.unary = opp_parse_order2(parser);

		return value;
	}
	else if (parser->lex->t.id == TMUL)
	{
		value = opp_new_node(parser, EDEREF);
		opp_next(parser->lex);
		value->defer_expr.defer = opp_parse_order2(parser);

		return value;
	}
	else if (parser->lex->t.id == TADDR)
	{
		value = opp_new_node(parser, EADDR);
		opp_next(parser->lex);
		value->addr_expr.addr = opp_parse_order2(parser);

		return value;
	}
	else if (parser->lex->t.id == TSIZEOF)
	{
		value = opp_new_node(parser, ESIZEOF);
		opp_next(parser->lex);
		value->sizeof_expr.size = opp_parse_order2(parser);

		return value;
	}

	return opp_parse_prefix(parser);
}

static struct Opp_Node* opp_parse_prefix(struct Opp_Parser* parser)
{
	struct Opp_Node* left = opp_parse_unary(parser);
	struct Opp_Node* result = NULL;

	opp_next(parser->lex);
	int operator = parser->lex->t.id;

	while (operator == TOPENP || operator == TOPENB ||
		operator == TDOT || operator == TDECR || 
		operator == TINCR)
	{
		switch (operator)
		{
			case TOPENP: {
				result = opp_new_node(parser, ECALL);
				result->call_expr.callee = left;
				result->call_expr.args = opp_parse_args(parser);

				opp_next(parser->lex);
				left = result;

				break;
			}

		}
		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Node* opp_parse_unary(struct Opp_Parser* parser)
{
	struct Opp_Node* unary = NULL;

	if (parser->lex->t.id == TOPENP) {
		return opp_parse_paran(parser);
	}

	unary = opp_new_node(parser, EUNARY);

	switch (parser->lex->t.id)
	{
		case TIDENT: 
			opp_parse_identifier(parser, unary);
			break;

		case TINTEGER:	
			opp_parse_integer(parser, unary);
			break;

		case TFLOAT:
			opp_parse_float(parser, unary);
			break;

		case TSTR:
			opp_parse_str(parser, unary);
			break;

		default:
			unary = NULL;
			break;
	}

	return unary;
}

static void opp_parse_identifier(struct Opp_Parser* parser, struct Opp_Node* node)
{
	node->unary_expr.type = TIDENT;

	if (parser->lex->t.buffer.len > 32)
		opp_error(parser->lex, "Identifier declaration exceeds 32 characters '%s'", parser->lex->t.buffer.buf);

	node->unary_expr.val.strval = (char*)malloc(parser->lex->t.buffer.len + 1);

	if (node->unary_expr.val.strval == NULL)
		INTERNAL_ERROR("Malloc fail");

	strcpy(node->unary_expr.val.strval, parser->lex->t.buffer.buf);
}

static void opp_parse_integer(struct Opp_Parser* parser, struct Opp_Node* node)
{
	node->unary_expr.type = TINTEGER;
	node->unary_expr.val.i64val = parser->lex->t.num;
}

static void opp_parse_float(struct Opp_Parser* parser, struct Opp_Node* node)
{
	opp_error(parser->lex, "Opp does not support floating point numbers at the moment...");
	node->unary_expr.type = TFLOAT;
	node->unary_expr.val.f64val = parser->lex->t.real;
}

static void opp_parse_str(struct Opp_Parser* parser, struct Opp_Node* node)
{
	node->unary_expr.type = TSTR;
	node->unary_expr.val.strval = (char*)malloc(parser->lex->t.buffer.len + 1);

	if (node->unary_expr.val.strval == NULL)
		INTERNAL_ERROR("Malloc fail");

	strcpy(node->unary_expr.val.strval, parser->lex->t.buffer.buf);
}

static struct Opp_Node* opp_parse_paran(struct Opp_Parser* parser)
{
	struct Opp_Node* node = NULL;
	opp_next(parser->lex);

	node = opp_parse_allign(parser);

	if (parser->lex->t.id != TCLOSEP) 
		opp_expect_error(parser, ')');

	return node;
}