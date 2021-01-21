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

static void opp_add_std_types(struct Opp_Parser* parser);

struct Opp_Parser* opp_parser_init(struct Opp_Scan* s)
{
	struct Opp_Parser* parser = (struct Opp_Parser*)
		malloc(sizeof(struct Opp_Parser));

	parser->lex = s;
	parser->nstmts = 0;
	parser->statments = calloc(64, sizeof(*parser->statments));
	parser->stmt_size = 64;

	if (parser->statments == NULL)
		goto err;

	// Init type tree
	parser->tree.types = (struct Opp_Type_Entry**)
		malloc(sizeof(struct Opp_Type_Entry*)*DEFAULT_TYPE_TREE_SIZE);

	if (parser->tree.types == NULL)
		goto err;

	parser->tree.size = DEFAULT_TYPE_TREE_SIZE;

	memset(parser->tree.types, 0, sizeof(struct Opp_Type_Entry*)*DEFAULT_TYPE_TREE_SIZE);

	opp_add_std_types(parser);

	return parser;

err:
	INTERNAL_ERROR("Malloc Fail");
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

static struct Opp_Type_Entry* add_type(struct Opp_Type_Tree* tree, char* name)
{
	unsigned int loc = hash_str(name, tree->size);

	if (tree->types[loc] != NULL) {
		struct Opp_Type_Entry* pos = tree->types[loc];

		while (pos) {
			if (!strcmp(pos->id, name)) 
				return NULL;
			else if (pos->next == NULL) {
				pos->next = (struct Opp_Type_Entry*)
					malloc(sizeof(struct Opp_Type_Entry));

				if (!pos->next)
					INTERNAL_ERROR("Malloc fail");

				pos->next->id = name;
				pos->next->next = NULL;
				return pos->next;
			}
			else 
				pos = pos->next;
		}
	}
	else {
		struct Opp_Type_Entry* t = (struct Opp_Type_Entry*)
			malloc(sizeof(struct Opp_Type_Entry));

		if (t == NULL)
			INTERNAL_ERROR("Malloc fail");

		t->id = name;
		t->next = NULL;
		tree->types[loc] = t;

		return t;
	}

	return NULL;
}

struct Opp_Type_Entry* get_type(struct Opp_Type_Tree* tree, char* name)
{
	unsigned int loc = hash_str(name, tree->size);

	struct Opp_Type_Entry* scope = tree->types[loc];

	if (scope != NULL) 
	{
		while (scope) {
			if (!strcmp(scope->id, name)) 
				return scope;
			else if (scope->next != NULL)
				scope = scope->next;
			else 
				return NULL;
		}
	}

	return NULL;
}

static void opp_add_std_types(struct Opp_Parser* parser)
{
	struct Opp_Type_Entry* type1;

	type1 = add_type(&parser->tree, "char");
	type1->t_type = TYPE_I8;
	char_type = type1;

	type1 = add_type(&parser->tree, "short");
	type1->t_type = TYPE_I16;

	type1 = add_type(&parser->tree, "int");
	type1->t_type = TYPE_I32;
	int_type = type1;

	type1 = add_type(&parser->tree, "long");
	type1->t_type = TYPE_I64;

	type1 = add_type(&parser->tree, "void");
	type1->t_type = TYPE_VOID;
	void_type = type1;

	type1 = add_type(&parser->tree, "float");
	type1->t_type = TYPE_FLOAT;

	type1 = add_type(&parser->tree, "double");
	type1->t_type = TYPE_DOUBLE;
	float_type = type1;
}

static bool opp_parse_atype(struct Opp_Parser* parser, struct Opp_Type_Decl* decl)
{
	opp_next(parser->lex);

	if (parser->lex->t.id == TUNSIGNED) {
		decl->unsign = 1;
		opp_next(parser->lex);
	}

	struct Opp_Type_Entry* t = get_type(&parser->tree, parser->lex->t.buffer.buf);

	if (t == NULL)
		return false;

	decl->decl = t;
	opp_next(parser->lex);
	while (parser->lex->t.id == TMUL) {
		decl->depth++;
		opp_next(parser->lex);
	}

	return true;
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

static struct Opp_Node* opp_parse_global_def(struct Opp_Parser* parser)
{
	switch (parser->lex->t.id)
	{
		// Structs / Extern etc...
		case TUNSIGNED:
		case TIDENT:
			return opp_parse_type(parser);

		case TEXTERN:
			return opp_parse_extern(parser);

		case TIMPORT:
			return opp_parse_import(parser);

		case TSTRUCT:
			return opp_parse_struct(parser);

		default:
			opp_error(parser->lex, 
				"Error unexpected global declaration '%s'", parser->lex->t.buffer.buf);
	}

	return NULL;
}

static struct Opp_Node* opp_parse_type(struct Opp_Parser* parser)
{
	struct Opp_Type_Decl type = {0};

	if (parser->lex->t.id == TUNSIGNED) {
		opp_next(parser->lex);
		type.unsign = 1;
	}

	type.decl = get_type(&parser->tree, parser->lex->t.buffer.buf);

	if (type.decl == NULL)
		opp_error(parser->lex, "Use of undeclared type '%s'", parser->lex->t.buffer.buf);

	if (type.decl->t_type == TYPE_STRUCT && type.unsign) {
		// printf("===>%s\n", type.decl->id);
		opp_error(parser->lex, "Struct '%s' type cannot have unsigned attribute", type.decl->id);
	}
	// type.decl->id = (char*)malloc(strlen(parser->lex->t.buffer.buf)+1);
	// strcpy(type.decl->id, parser->lex->t.buffer.buf);

	int lookahead = 1;
	do {
		opp_peek_tok(parser->lex, lookahead);
		if (parser->lex->t.id != TMUL && parser->lex->t.id != TIDENT)
			opp_error(parser->lex, "Unexpected token in type definition");
		lookahead++;
	} while (parser->lex->t.id != TIDENT);

	opp_peek_tok(parser->lex, lookahead);

	if (parser->lex->t.id == TOPENP)
		return opp_parse_func(parser, type);
	else
		return opp_parse_var_decl(parser, type);
	
	return NULL;
}

static struct Opp_Node* opp_parse_func(struct Opp_Parser* parser, struct Opp_Type_Decl type)
{
	struct Opp_Node* func_node = opp_new_node(parser, STMT_FUNC);
	
	func_node->fn_stmt.type = type; 

	opp_next(parser->lex);

	while (parser->lex->t.id != TIDENT) {
		func_node->fn_stmt.type.depth++;
		opp_next(parser->lex);
	}
	
	struct Opp_Node* name = opp_parse_unary(parser);
	func_node->fn_stmt.name = name;
	opp_next(parser->lex);

	if (parser->lex->t.id != TOPENP)
		opp_error(parser->lex, "Expected '(' after function identifier '%s'",
			name->unary_expr.val.strval);

	func_node->fn_stmt.args = (struct Opp_Node*)
		malloc(sizeof(struct Opp_Node)*DEFAULT_LIST_SIZE);

	if (func_node->fn_stmt.args == NULL)
		INTERNAL_ERROR("Malloc fail");

	unsigned int i = 0;

	do {
		opp_next(parser->lex);
		if (parser->lex->t.id == TCLOSEP 
			|| parser->lex->t.id == FEND)
			break;

		if (i == DEFAULT_LIST_SIZE)
			opp_error(parser->lex, "Max function parameters met");
		
		if (parser->lex->t.id == TUNSIGNED) {
			func_node->fn_stmt.args[i].var_stmt.type.unsign = 1;
			opp_next(parser->lex);
		}

		func_node->fn_stmt.args[i].type = STMT_VAR;
		func_node->fn_stmt.args[i].debug = func_node->debug;
		struct Opp_Type_Entry* type = get_type(&parser->tree, parser->lex->t.buffer.buf);

		if (type == NULL)
			opp_error(parser->lex, "Use of undefined type '%s' in function argument #%d",
				parser->lex->t.buffer.buf, i+1);

		func_node->fn_stmt.args[i].var_stmt.type.decl = type;

		opp_next(parser->lex);
		while (parser->lex->t.id != TIDENT) {
			func_node->fn_stmt.args[i].var_stmt.type.depth++;
			opp_next(parser->lex);
		}
	
		func_node->fn_stmt.args[i].var_stmt.var = opp_parse_allign(parser);

		i++;

	} while (parser->lex->t.id == TCOMMA);


	func_node->fn_stmt.len = i;

	if (parser->lex->t.id != TCLOSEP)
		opp_error(parser->lex, "Expected ')' after parameter list in function declaration");

	opp_next(parser->lex);

	if (parser->lex->t.id == TSEMICOLON)
		func_node->fn_stmt.body = NULL;
	else
		func_node->fn_stmt.body = opp_parse_statement(parser);

	return func_node;
}

static struct Opp_Node* opp_parse_struct(struct Opp_Parser* parser)
{
	struct Opp_Node* struct_node = opp_new_node(parser, STMT_STRUCT);

	opp_next(parser->lex);

	if (parser->lex->t.id != TIDENT)
		opp_error(parser->lex, "Expected a identifier after struct");

	struct_node->struct_stmt.name = (char*)malloc(strlen(parser->lex->t.buffer.buf)+1);
	strcpy(struct_node->struct_stmt.name, parser->lex->t.buffer.buf);

	struct Opp_Type_Entry* entry = add_type(&parser->tree, struct_node->struct_stmt.name);

	if (entry == NULL)
		opp_error(parser->lex, "Redefinition of type '%s'", struct_node->struct_stmt.name);
	entry->t_type = TYPE_STRUCT;
	entry->s_type = init_namespace(NULL, malloc);
	entry->s_elems = &struct_node->struct_stmt;
	struct_node->struct_stmt.ns = entry->s_type;

	opp_next(parser->lex);
	// add peek to see if non typedef declaration aka struct test a; for example

	if (parser->lex->t.id != TOPENC)
		opp_error(parser->lex, "Expected a '{' after 'struct %s'", struct_node->struct_stmt.name);

	struct_node->struct_stmt.elems = (struct Opp_Node**)
		malloc(sizeof(struct Opp_Node*)*16);

	memset(struct_node->struct_stmt.elems, 0, sizeof(struct Opp_Node*)*16);

	unsigned int i = 0;
	do {
		opp_next(parser->lex);
		if (parser->lex->t.id == FEND || parser->lex->t.id == TCLOSEC)
			break;

		if (i == 16)
			opp_error(parser->lex, "TODO REALLOC STRUCT");

		struct_node->struct_stmt.elems[i] = opp_parse_type(parser);

		i++;

	} while (parser->lex->t.id != TCLOSEP);

	opp_next(parser->lex);
	if (parser->lex->t.id != TSEMICOLON)
		opp_error(parser->lex, "Expected ';' after struct '%s' declaration", struct_node->struct_stmt.name);

	struct_node->struct_stmt.len = i;

	return struct_node;
}

static struct Opp_Node* opp_parse_statement(struct Opp_Parser* parser)
{
	switch (parser->lex->t.id)
	{
		case TSEMICOLON: // add pass through
			return NULL;
			
		case TOPENC:
			return opp_parse_block(parser);

		case TRET:
			return opp_parse_return(parser);

		case TWHILE:
			return opp_parse_while(parser);

		case TGOTO:
			return opp_parse_goto(parser);

		case TIF:
			return opp_parse_if(parser);

		case TFOR:
			return opp_parse_for(parser);

		case TSWITCH:
			return opp_parse_switch(parser);

		case TCASE:
			return opp_parse_case(parser);

		case TBREAK:
			return opp_parse_break(parser);

		case TUNSIGNED:
			return opp_parse_type(parser);

		case TIDENT: 
			if (opp_parse_peak(parser, ':'))
				return opp_parse_label(parser);

			if (get_type(&parser->tree, parser->lex->t.buffer.buf) != NULL)
				return opp_parse_type(parser);
			else 
				return opp_parse_expr(parser);

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
		opp_error(parser->lex, "Expected terminating '}' after block statement\n");

	return block;
}

static struct Opp_Node* opp_parse_label(struct Opp_Parser* parser)
{
	struct Opp_Node* label = opp_new_node(parser, STMT_LABEL);

	label->label_stmt.name = opp_parse_unary(parser);
	opp_next(parser->lex);

	return label;
}

static struct Opp_Node* opp_parse_for(struct Opp_Parser* parser)
{
	struct Opp_Node* for_loop = opp_new_node(parser, STMT_FOR);

	opp_next(parser->lex);

	if (parser->lex->t.id != TOPENP)
		opp_error(parser->lex, "Expectd '(' after 'for'");

	opp_next(parser->lex);
	for_loop->for_stmt.decl = opp_parse_statement(parser);
	opp_next(parser->lex);
	for_loop->for_stmt.cond = opp_parse_expr(parser);
	opp_next(parser->lex);
	for_loop->for_stmt.expr = opp_parse_allign(parser);

	if (parser->lex->t.id != TCLOSEP)
		opp_error(parser->lex, "Expected ')' after 'for' statement");

	opp_next(parser->lex);

	for_loop->for_stmt.body = opp_parse_statement(parser);

	return for_loop;
}

static struct Opp_Node* opp_parse_case(struct Opp_Parser* parser)
{
	struct Opp_Node* cond = opp_new_node(parser, STMT_CASE);

	opp_next(parser->lex);

	cond->case_stmt.cond = opp_parse_allign(parser);

	if (parser->lex->t.id != TCOLON)
		opp_error(parser->lex, "Expected colon after 'case' keyword");

	opp_next(parser->lex);
	cond->case_stmt.stmt = opp_parse_statement(parser);

	return cond;
}

static struct Opp_Node* opp_parse_switch(struct Opp_Parser* parser)
{
	struct Opp_Node* swt = opp_new_node(parser, STMT_SWITCH);

	opp_next(parser->lex);

	if (parser->lex->t.id != TOPENP)
		opp_error(parser->lex, "Expected '(' after 'switch' statement");

	opp_next(parser->lex);
	swt->switch_stmt.cond = opp_parse_allign(parser);

	if (parser->lex->t.id != TCLOSEP)
		opp_error(parser->lex, "Expected ')' after 'switch' condition");

	opp_next(parser->lex);

	if (parser->lex->t.id != TOPENC)
		opp_error(parser->lex, "Expected '{' after 'switch' condition\n\tFormat: switch (...) { \n\t\t\t...\n\t\t}");

	swt->switch_stmt.block = opp_parse_block(parser);

	return swt;
}

static struct Opp_Node* opp_parse_break(struct Opp_Parser* parser)
{
	struct Opp_Node* brk = opp_new_node(parser, STMT_BREAK);

	opp_next(parser->lex);

	return brk;
}

static struct Opp_Node* opp_parse_var_decl(struct Opp_Parser* parser, struct Opp_Type_Decl type)
{
	struct Opp_Node* var = opp_new_node(parser, STMT_VAR);

	var->var_stmt.type = type;

	opp_next(parser->lex);
	while (parser->lex->t.id != TIDENT) {
		var->var_stmt.type.depth++;
		opp_next(parser->lex);
	}
	
	var->var_stmt.var = opp_parse_allign(parser);
	//try tree recursion logic with ptr to next
	// var->var_stmt.vars = opp_parse_comma(parser);

	if (parser->lex->t.id != TSEMICOLON)
		opp_error(parser->lex, "Expected ';' after auto list initializaion");

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

	extrn->extrn_stmt.stmt = opp_parse_type(parser);

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
	ifstmt->if_stmt.other = NULL;

	opp_peek_tok(parser->lex, 1);

	if (parser->lex->t.id == TELSE) {
		opp_next(parser->lex);
		opp_next(parser->lex);
		ifstmt->if_stmt.other = opp_parse_statement(parser);
	}

	return ifstmt;
}

static struct Opp_Node* opp_parse_expr(struct Opp_Parser* parser)
{
	struct Opp_Node* new_expr = opp_parse_allign(parser);

	if (parser->lex->t.id != TSEMICOLON) 
		opp_error(parser->lex, "Expected ';' after expression");

	return new_expr;
}

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
		if (i == DEFAULT_LIST_SIZE)
			opp_error(parser->lex, "Max auto list variable declaration limit met (8)");

		opp_next(parser->lex);

		if (parser->lex->t.id == FEND || parser->lex->t.id == TCLOSEP)
			break;

		list->list[i] = opp_parse_allign(parser);

		i++;
	} while (parser->lex->t.id == TCOMMA);

	list->length = i;

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

	while (operator == TEQ || operator == TCOLON)
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
	// struct Opp_Node* left = opp_parse_order1(parser);
	struct Opp_Node* left = opp_parse_shifts(parser);
	struct Opp_Node* right = NULL;
	struct Opp_Node* comp = NULL;

	int operator = parser->lex->t.id;

	while (operator == TGT || operator == TLT ||
			operator == TGE || operator == TLE)
	{
		opp_next(parser->lex);
		// right = opp_parse_order1(parser);
		right = opp_parse_shifts(parser);

		comp = opp_new_node(parser, ELOGIC);
		comp->logic_expr.tok = operator;
		comp->logic_expr.left = left;
		comp->logic_expr.right = right;

		left = comp;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Node* opp_parse_shifts(struct Opp_Parser* parser)
{
	struct Opp_Node* left = opp_parse_order1(parser);
	struct Opp_Node* right = NULL;
	struct Opp_Node* result = NULL;

	int operator = parser->lex->t.id;

	while (operator == TSHL || operator == TSHR)
	{
		opp_next(parser->lex);
		right = opp_parse_order1(parser);

		result = opp_new_node(parser, EBIT);
		result->bin_expr.tok = operator;
		result->bin_expr.left = left;
		result->bin_expr.right = right;

		left = result;

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
		value->deref_expr.deref = opp_parse_order2(parser);

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
		assert(0);
		// value = opp_new_node(parser, ESIZEOF);
		// value->sizeof_expr.size.depth = 0;
		// value->sizeof_expr.size.size = 0;
		// value->sizeof_expr.size.unsign = 0;
		// opp_next(parser->lex);

		// if (parser->lex->t.id == TUNSIGNED) {
		// 	value->sizeof_expr.size.unsign = 1;
		// 	opp_next(parser->lex);
		// }

		// struct Opp_Type_Entry* t = get_type(&parser->tree, parser->lex->buffer.buf);

		// if (t == NULL)
		// 	opp_error(parser->lex, "Expected valid type after sizeof instead got '%s'",
		// 		parser->lex->buffer.buf);

		// value->sizeof_expr.size.decl = t;
		// value->sizeof_expr.size = opp_parse_order2(parser);

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

				result->call_expr.args = opp_parse_comma(parser);

				if (parser->lex->t.id != TCLOSEP)
					opp_error(parser->lex, "Expected ')' in function call");

				opp_next(parser->lex);
				left = result;
				break;
			}

			case TOPENB: {
				result = opp_new_node(parser, EELEMENT);
				result->elem_expr.name = left;
				opp_next(parser->lex);
				result->elem_expr.loc = opp_parse_allign(parser);

				opp_next(parser->lex);
				left = result;
				break;
			}

			case TDOT: {
				result = opp_new_node(parser, EDOT);
				result->dot_expr.left = left;
				opp_next(parser->lex);
				result->dot_expr.right = opp_parse_unary(parser);
				opp_next(parser->lex);

				left = result;
				break;
			}

			case TDECR: case TINCR: {
				result = opp_new_node(parser, EADJUST);
				result->adjust_expr.left = left;
				result->adjust_expr.type = operator;

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

		case TCH:
			unary->unary_expr.type = TINTEGER;
			unary->unary_expr.val.i64val = (int64_t)parser->lex->t.buffer.buf[0];
			break;
			
		default:
			opp_error(parser->lex, "Expected unary value but got unexpected");
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