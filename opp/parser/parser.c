/** @file parser.c
 * 
 * @brief Opp Parser
 *      
 * Copyright (c) 2021 Maks S
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
#include "../ast/ast.h"
#include "../util/util.h"
#include "../memory/memory.h"
#include "types.h"
#include "../lexer/lexer.h"

// Util
static struct Opp_Stmt* opp_new_stmt(struct Opp_Parser* parser, enum Opp_Stmt_Type type);
static struct Opp_Expr* opp_new_expr(struct Opp_Parser* parser, enum Opp_Expr_Type type);

static struct Opp_Type* opp_parse_declaration_specifier(struct Opp_Parser* parser);
static struct Opp_Stmt* opp_parse_global_def(struct Opp_Parser* parser);
static struct Opp_Stmt* opp_parse_decl(struct Opp_Parser* parser);
static struct Opp_Type* opp_parse_declarator(struct Opp_Parser* parser, struct Opp_Type* base, char** name);
static struct Opp_Type* opp_parse_post_decl(struct Opp_Parser* parser, struct Opp_Type* base, char** name);
static struct Opp_Type* opp_parse_param(struct Opp_Parser* parser, struct Opp_Type* base);
static struct Opp_Type* opp_parse_array(struct Opp_Parser* parser, struct Opp_Type* base);

// Statements
static struct Opp_Stmt* opp_parse_stmt(struct Opp_Parser* parser);
static struct Opp_Stmt* opp_parse_definition(struct Opp_Parser* parser, struct Opp_Type* type, bool isdef, char* name);
static struct Opp_Stmt* opp_decl(struct Opp_Parser* parser, struct Opp_Type* type, char* name);
static struct Opp_Stmt* opp_func_decl(struct Opp_Parser* parser, struct Opp_Type* type, char* name);
static struct Opp_Stmt* opp_typedef(struct Opp_Parser* parser, struct Opp_Type* type, char* name);
static struct Opp_Stmt* opp_parse_block(struct Opp_Parser* parser);
static struct Opp_Stmt* opp_parse_stmt_expr(struct Opp_Parser* parser);
static struct Opp_Stmt* opp_parse_if(struct Opp_Parser* parser);
static struct Opp_Stmt* opp_parse_while(struct Opp_Parser* parser);
static struct Opp_Stmt* opp_parse_do_while(struct Opp_Parser* parser);
static struct Opp_Stmt* opp_parse_for(struct Opp_Parser* parser);

// Expressions
static struct Opp_Expr* opp_parse_comma(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_assign(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_ternary(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_or(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_and(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_bit_or(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_bit_xor(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_bit_and(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_relation(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_relation2(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_shift(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_op(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_op2(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_cast(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_prefix(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_postfix(struct Opp_Parser* parser);
static struct Opp_Expr* opp_parse_unary(struct Opp_Parser* parser);

struct Opp_Parser* opp_init_parser(struct Opp_Scan* s)
{
    struct Opp_Parser* parser = (struct Opp_Parser*)
		opp_alloc(sizeof(struct Opp_Parser));

    if (!parser)
        goto err;

	parser->lex = s;
	parser->nstmts = 0;
	parser->statements = (struct Opp_Stmt**)
        opp_alloc(sizeof(struct Opp_Stmt*)*64);
	parser->allocated = 64;

	if (!parser->statements)
		goto err;
    
    memset(parser->statements, 0, sizeof(struct Opp_Stmt*)*64);

    parser->state.global = opp_create_map(TYPE_MAP_SIZE, NULL);
    parser->state.scope = parser->state.global;

	return parser;

err:
	MALLOC_FAIL();
	return NULL;
}

static struct Opp_Stmt* opp_new_stmt(struct Opp_Parser* parser, 
									enum Opp_Stmt_Type type)
{
    struct Opp_Stmt* node = (struct Opp_Stmt*)
    	opp_alloc(sizeof(struct Opp_Stmt));

	if (!node)
		MALLOC_FAIL();

	node->type = type;
	node->debug.line = parser->lex->line;
	node->debug.colum = parser->lex->colum;

	return node;
}

static struct Opp_Expr* opp_new_expr(struct Opp_Parser* parser, 
									enum Opp_Expr_Type type)
{
	struct Opp_Expr* node = (struct Opp_Expr*)
		opp_alloc(sizeof(struct Opp_Expr));

	if (!node)
		MALLOC_FAIL();

	node->type = type;
	node->debug.line = parser->lex->line;
	node->debug.colum = parser->lex->colum;
	node->lvalue = false;

	return node;
}

void opp_parser_begin(struct Opp_Parser* parser)
{
    for (;;) {
		opp_next(parser->lex);
		if (parser->lex->t.id == FEND)
			break;

		struct Opp_Stmt* stmt = opp_parse_global_def(parser);

		if (parser->nstmts == parser->allocated) {
			parser->statements = (struct Opp_Stmt**)
				opp_realloc(parser->statements, 
					(64+parser->allocated) * sizeof(struct Opp_Stmt*), 
					sizeof(struct Opp_Stmt*)*parser->allocated);

			if (!parser->statements)
				MALLOC_FAIL();

			parser->allocated += 64;
		}

		parser->statements[parser->nstmts] = stmt;
		parser->nstmts++;
	}
}

static bool is_valid_def(enum Opp_Token t)
{
	switch (t) {
		case TEXTERN:    case TSTATIC: 
		case TINLINE:    case TUNION:   case TENUM:     
		case TSTRUCT:    case TCHAR:    case TSHORT:
		case TINT:       case TLONG:    case TUNSIGNED:
		case TSIGNED:    case TCONST:   case TVOLATILE:
		case TFLOAT:     case TDOUBLE:  case TVOID:
		case TREGITSTER: case TAUTO:
			return true;

		default:
			return false;
	}
}

static struct Opp_Stmt* opp_parse_global_def(struct Opp_Parser* parser)
{
	switch (parser->lex->t.id)
	{
		case TIDENT:     case TEXTERN:  case TSTATIC: 
		case TINLINE:    case TUNION:   case TENUM:     
		case TSTRUCT:    case TCHAR:    case TSHORT:
		case TINT:       case TLONG:    case TUNSIGNED:
		case TSIGNED:    case TCONST:   case TVOLATILE:
		case TFLOAT:     case TDOUBLE:  case TVOID:
		case TREGITSTER: case TAUTO:    case TTYPEDEF:
			return opp_parse_decl(parser);

		default:
			opp_error(parser->lex, "Error unexpected global declaration '%s'", 
				tok_to_str(parser->lex));

	}
	return NULL;
}

#define ANOTHER() \
	opp_error(parser->lex, "Another attribute found in declaration '%s'", parser->lex->t.buffer.buf)

static void opp_parse_struct_or_union(struct Opp_Parser* parser, struct Opp_Type* type)
{

	/* 
		add support for 
		union {

		} a;

		A ident can either reference a type declared inside type map or it can have it own copy so to say
		^^^ might need flag for union

	*/
	enum Opp_Type_T t = parser->lex->t.id == TSTRUCT ? TYPE_STRUCT : TYPE_UNION;

	opp_next(parser->lex);

	if (parser->lex->t.id != TIDENT)
		opp_error(parser->lex, "Expected indentifier when declaring struct/union");
	
	type->type = t;
	type->val.obj.is_complete = false;

	enum Opp_Token next = opp_peek_tok(parser->lex, 1);

	if (next == TOPENC) {
		type->val.obj.is_complete = true;
		while (parser->lex->t.id != TCLOSEC) {
			opp_next(parser->lex);
			if (parser->lex->t.id == FEND)
				opp_error(parser->lex, "Found end of file while inside of struct/union declaration");
		}
	}
}

static struct Opp_Type* opp_parse_declaration_specifier(struct Opp_Parser* parser)
{
	struct Opp_Type* type = opp_create_type(TYPE_NONE, NULL);
	bool got_base = false;
	bool got_mod  = false;
	bool got_sign = false;
	enum Opp_Type_T mod;

	for (;;) {
		switch (parser->lex->t.id) {
			case FEND:
				opp_error(parser->lex, "Unexpected end of file in declaration");

			case TEXTERN: case TSTATIC: 
			case TAUTO:   case TREGITSTER: 
			case TTYPEDEF: {
				if (type->storage_class != 0)
					ANOTHER();
				type->storage_class = parser->lex->t.id;
				break;
			}

			case TSTRUCT: case TUNION:
				if (got_mod || got_sign || got_base)
					opp_error(parser->lex, "Unexpected '%s' in declaration", tok_to_str(parser->lex));
				opp_parse_struct_or_union(parser, type);
				break;

			case TENUM: assert(false); break;

		    case TCHAR: {
		    	if (got_base)
		    		ANOTHER();
		    	type->type = TYPE_CHAR;
		    	got_base = true;
		    	break;
		    }

		    case TINT: {
		    	if (got_base)
		    		ANOTHER();
		    	type->type = TYPE_INT;
		    	got_base = true;
		    	break;
		    }
		    
		    case TFLOAT: {
		    	if (got_base)
		    		ANOTHER();
		    	type->type = TYPE_FLOAT;
		    	got_base = true;
		    	break;
		    }

		    case TDOUBLE: {
		    	if (got_base)
		    		ANOTHER();
		    	type->type = TYPE_DOUBLE;
		    	got_base = true;
		    	break;
		    }

			case TVOID: {
				if (got_base)
		    		ANOTHER();
		    	type->type = TYPE_VOID;
		    	got_base = true;
		    	break;
		    }

		    case TLONG: case TSHORT: {
		    	if (got_mod)
		    		ANOTHER();
		    	
		    	mod = parser->lex->t.id == TLONG 
		    		? TYPE_LONG : TYPE_SHORT;

		    	got_mod = true;
				break;
			}

			case TRESTRICT: {
				type->restrict_attr = true; 
				break;
			}

			case TINLINE: {
				type->inline_attr = true; 
				break;
			}

			case TVOLATILE: {
				type->volatile_attr = true; 
				break;
			}

			case TCONST: {
				type->const_attr = true; 
				break;
			}

			case TUNSIGNED: case TSIGNED: {
				if (got_sign)
					ANOTHER();
				got_sign = true;
				type->unsign = parser->lex->t.id == TUNSIGNED;
				break;
			}

			case TIDENT: {
				struct Opp_Bucket* b = opp_get_bucket(parser->state.scope, 
					parser->lex->t.buffer.buf);
				if (b) {
					if (got_base)
						ANOTHER();
					got_base = true;
					int st = type->storage_class;
					*type = *(struct Opp_Type*)b->data;
					type->storage_class = st;
				}
				else {
					goto end;
				}
				break;
			}

			default: {
				goto end;
			}
		}
		opp_next(parser->lex);
	}

end:
	if (got_mod) {
		if (!got_base) {
			type->type = mod;
		}
		else {
			switch (type->type) {
				case TYPE_INT:
					type->type = mod;
					break;
				case TYPE_DOUBLE:
					if (mod == TYPE_LONG)
						type->type = TYPE_LDOUBLE;
					else
						ANOTHER();
					break;
				default:
					opp_error(parser->lex, "Unexpected extra modifier in declaration");
					break;
			}
		}
	}

	return type;
}

static struct Opp_Type* opp_parse_array(struct Opp_Parser* parser, struct Opp_Type* base)
{
	return NULL;
}

static struct Opp_Type* opp_parse_param(struct Opp_Parser* parser, struct Opp_Type* base)
{
	base = opp_create_type(TYPE_FUNC, base);
	base->val.fn.param = (struct Opp_Func_Mem*)opp_alloc(sizeof(struct Opp_Func_Mem)*6);
	if (!base->val.fn.param)
		MALLOC_FAIL();
	unsigned int len = 0;

	opp_next(parser->lex);
	while (parser->lex->t.id != TCLOSEP) {
		char* name = NULL;
		if (len == 6)
			opp_error(parser->lex, "Max func param length met (6)");
		if (parser->lex->t.id == TVA_ARGS) {
			base->val.fn.is_vaarg = true;
			opp_next(parser->lex);
			break;
		}

		if (!is_valid_def(parser->lex->t.id))
			opp_error(parser->lex, "Invalid definition in func paramter #%d", len);

		struct Opp_Type* type = opp_parse_declaration_specifier(parser);

		if (type->storage_class == TEXTERN || type->storage_class == TSTATIC)
			opp_error(parser->lex, "Unexpected storage class in function paramater");

		if (type->type == TYPE_NONE)
			type->type = TYPE_INT;

		type = opp_parse_declarator(parser, type, &name);

		base->val.fn.param[len].name = name;
		base->val.fn.param[len].type = type;

		if (parser->lex->t.id == TCOMMA)
			opp_next(parser->lex);
		len++;
	}

	if (parser->lex->t.id != TCLOSEP)
		opp_error(parser->lex, "Expected ')' in function declaration");

	base->val.fn.len = len;

	return base;
}

static struct Opp_Type* opp_parse_post_decl(struct Opp_Parser* parser, struct Opp_Type* base, char** name)
{
	struct Opp_Type* nest = NULL;
	if (parser->lex->t.id == TOPENP) { // check if not func
		opp_next(parser->lex);
		nest = opp_parse_declarator(parser, nest, name);

		if (parser->lex->t.id != TCLOSEP)
			opp_error(parser->lex, "Expected closing ')'");
		opp_next(parser->lex);
	}
	else if (parser->lex->t.id == TIDENT) {
		*name = cpy_string(parser->lex->t.buffer.buf);
		opp_next(parser->lex);
	}

	while (parser->lex->t.id == TOPENP || parser->lex->t.id == TOPENB) {
		if (parser->lex->t.id == TOPENP) {
			base = opp_parse_param(parser, base);
			opp_next(parser->lex);
		}
		else {
			opp_next(parser->lex);
			opp_next(parser->lex);
		}
	}

	if (nest != NULL) {
		base = opp_type_fix(nest, base);
	}

	return base;
}

static struct Opp_Type* opp_parse_pointer(struct Opp_Parser* parser, struct Opp_Type* base, char** name)
{
	while (parser->lex->t.id == TMUL) {
		base = opp_create_type(TYPE_PTR, base);
		opp_next(parser->lex);

		while (parser->lex->t.id == TCONST || 
			   parser->lex->t.id == TRESTRICT ||
			   parser->lex->t.id == TVOLATILE) 
		{
			if (parser->lex->t.id == TCONST)
				base->const_ptr = 1;
			else if (parser->lex->t.id == TRESTRICT)
				base->restrict_ptr = 1;
			else if (parser->lex->t.id == TVOLATILE)
				base->volatile_ptr = 1;

			opp_next(parser->lex);
		}
	}

	base = opp_parse_post_decl(parser, base, name);

	return base;
}

static struct Opp_Type* opp_parse_declarator(struct Opp_Parser* parser, struct Opp_Type* base, char** name)
{
	return opp_parse_pointer(parser, base, name);
}

static struct Opp_Stmt* opp_parse_definition(struct Opp_Parser* parser, struct Opp_Type* type, bool isdef, char* name)
{
	struct Opp_Stmt* stmt = NULL;
	if (isdef)
		stmt = opp_typedef(parser, type, name);
	else if (parser->lex->t.id == TOPENC) 
		stmt = opp_func_decl(parser, type, name);
	else 
		stmt = opp_decl(parser, type, name);

	return stmt;
}

static struct Opp_Stmt* opp_decl(struct Opp_Parser* parser, struct Opp_Type* type, char* name)
{
	if (!name && (type->type != TYPE_STRUCT && type->type != TYPE_UNION))
		opp_error(parser->lex, "Expected identifier in declaration");

	struct Opp_Stmt* decl = opp_new_stmt(parser, STMT_DECL);

	decl->stmt.decl.name = name;
	decl->stmt.decl.type = type;

	return decl;
}

static struct Opp_Stmt* opp_func_decl(struct Opp_Parser* parser, struct Opp_Type* type, char* name)
{
	if (!name)
		opp_error(parser->lex, "Expected identifier in function declaration");

	struct Opp_Stmt* func = opp_new_stmt(parser, STMT_FUNC);

	func->stmt.func.name = name;
	func->stmt.func.type = type;
	func->stmt.func.body = opp_parse_block(parser);

	return func;
}

static struct Opp_Stmt* opp_typedef(struct Opp_Parser* parser, struct Opp_Type* type, char* name)
{
	if (!name)
		opp_error(parser->lex, "Expected identifier in typedef declaration");

	struct Opp_Bucket* b = opp_create_bucket(parser->state.scope, name);

	if (!b)
		opp_error(parser->lex, "Attemping to redeclare type of name '%s'", name);

	b->data = (void*)type;

	return opp_new_stmt(parser, STMT_TYPEDEF);
}

static struct Opp_Stmt* opp_parse_decl(struct Opp_Parser* parser)
{
	char* name = NULL;
	struct Opp_Type* base = opp_parse_declaration_specifier(parser);
	bool is_typedef = base->storage_class == TTYPEDEF;
	struct Opp_Type* t = opp_parse_declarator(parser, base, &name);

	struct Opp_Stmt* stmt = opp_parse_definition(parser, t, is_typedef, name);
		
	opp_debug_type(t);

	if (stmt->type == STMT_FUNC)
		return stmt;

	struct Opp_Stmt* list = stmt;
	while (parser->lex->t.id == TCOMMA) {
		name = NULL;
		opp_next(parser->lex);
		t = opp_parse_declarator(parser, base, &name);
		list->stmt.decl.next = opp_parse_definition(parser, t, is_typedef, name);
		list = list->stmt.decl.next;
	}

	if (parser->lex->t.id != TSEMICOLON)
		opp_error(parser->lex, "Expected ';' after declaration");

	return stmt;
}

static struct Opp_Stmt* opp_parse_stmt(struct Opp_Parser* parser)
{
	switch (parser->lex->t.id)
	{
		case TIF:
			return opp_parse_if(parser);

		case TWHILE:
			return opp_parse_while(parser);

		case TDO:
			return opp_parse_do_while(parser);

		case TFOR:
			return opp_parse_for(parser);
			
		case TSWITCH:
		case TCASE:
		case TDEFAULT:
		case TRETURN:
		case TCONTINUE:
		break;

		case TSEMICOLON:
			return opp_new_stmt(parser, STMT_NOP);

		case TOPENB:
			return opp_parse_block(parser);

		case TIDENT:
			if (!opp_get_bucket(parser->state.scope, parser->lex->t.buffer.buf))
				break;
		case TUNION:     case TENUM:
		case TSTRUCT:    case TCHAR:    case TSHORT: 
		case TINT:       case TLONG:    case TUNSIGNED:
		case TSIGNED:    case TCONST:   case TVOLATILE:
		case TFLOAT:     case TDOUBLE:  case TVOID:
		case TREGITSTER: case TAUTO:    case TTYPEDEF:
			return opp_parse_decl(parser);

		default: break;
	}
	return opp_parse_stmt_expr(parser);
}

static struct Opp_Stmt* opp_parse_block(struct Opp_Parser* parser)
{
	struct Opp_Hashmap* old = parser->state.scope;
	parser->state.scope = opp_create_map(TYPE_MAP_SIZE, old);

	struct Opp_Stmt* block = opp_new_stmt(parser, STMT_BLOCK);
	unsigned int len = 0;

	block->stmt.block.stmt = (struct Opp_Stmt**)opp_alloc(sizeof(struct Opp_Stmt*)*16);

	if (!block->stmt.block.stmt)
		MALLOC_FAIL();

	opp_next(parser->lex);

	while (parser->lex->t.id != TCLOSEC) {
		if (parser->lex->t.id == FEND)
			break;

		block->stmt.block.stmt[len] = opp_parse_stmt(parser);
		len++;

		opp_next(parser->lex);
	}

	if (parser->lex->t.id != TCLOSEC)
		opp_error(parser->lex, "Expected terminating '}'");

	block->stmt.block.len = len;

	parser->state.scope = old;

	return block;
}

static struct Opp_Stmt* opp_parse_if(struct Opp_Parser* parser)
{
	struct Opp_Stmt* stmt = opp_new_stmt(parser, STMT_IF);

	opp_next(parser->lex);

	if (parser->lex->t.id != TOPENP)
		opp_error(parser->lex, "Expected '(' after if");

	opp_next(parser->lex);

	stmt->stmt.if_stmt.cond = opp_parse_comma(parser);

	if (parser->lex->t.id != TCLOSEP)
		opp_error(parser->lex, "Expected ')' after if condition");

	opp_next(parser->lex);

	stmt->stmt.if_stmt.then = opp_parse_stmt(parser);

	if (opp_peek_tok(parser->lex, 1) == TELSE) {
		opp_next(parser->lex);
		opp_next(parser->lex);
		stmt->stmt.if_stmt.other = opp_parse_stmt(parser);
	}

	return stmt;
}

static struct Opp_Stmt* opp_parse_while(struct Opp_Parser* parser)
{
	struct Opp_Stmt* stmt = opp_new_stmt(parser, STMT_WHILE);

	opp_next(parser->lex);

	if (parser->lex->t.id != TOPENP)
		opp_error(parser->lex, "Expected '(' after while");

	opp_next(parser->lex);

	stmt->stmt.while_stmt.cond = opp_parse_comma(parser);

	if (parser->lex->t.id != TCLOSEP)
		opp_error(parser->lex, "Expected ')' after while condition");

	opp_next(parser->lex);

	stmt->stmt.while_stmt.then = opp_parse_stmt(parser);

	return stmt;
}

static struct Opp_Stmt* opp_parse_do_while(struct Opp_Parser* parser)
{
	struct Opp_Stmt* stmt = opp_new_stmt(parser, STMT_DOWHILE);

	return stmt;
}

static struct Opp_Stmt* opp_parse_for(struct Opp_Parser* parser)
{
	struct Opp_Stmt* stmt = opp_new_stmt(parser, STMT_FOR);

	return stmt;
}

static struct Opp_Stmt* opp_parse_stmt_expr(struct Opp_Parser* parser)
{
	struct Opp_Stmt* expr = opp_new_stmt(parser, STMT_EXPR);

	expr->stmt.expr_stmt = opp_parse_comma(parser);

	if (parser->lex->t.id != TSEMICOLON)
		opp_error(parser->lex, "Expected ';' after expression");

	return expr;
}

static struct Opp_Expr* opp_parse_comma(struct Opp_Parser* parser)
{
	struct Opp_Expr* expr = opp_parse_assign(parser);
	struct Opp_Expr* e = expr;

	while (parser->lex->t.id == TCOMMA) {
		opp_next(parser->lex);
		e->next = opp_parse_assign(parser);
		e = e->next;
	}

	return expr;
}

static struct Opp_Expr* opp_parse_assign(struct Opp_Parser* parser)
{
	struct Opp_Expr* left = opp_parse_ternary(parser);
	struct Opp_Expr* right = NULL;
	struct Opp_Expr* node = NULL;

	int operator = parser->lex->t.id;

	while (operator == TEQ || operator == TADDEQ ||
		operator == TMINEQ || operator == TDIVEQ ||
		operator == TMULEQ || operator == TMODEQ ||
		operator == TSHLEQ || operator == TSHREQ ||
		operator == TANDEQ || operator == TOREQ)
	{
		opp_next(parser->lex);

		right = opp_parse_ternary(parser);

		node = opp_new_expr(parser, EXPR_ASSIGN);
		node->expr.bin.tok = operator;
		node->expr.bin.left = left;
		node->expr.bin.right = right;

		left = node;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Expr* opp_parse_ternary(struct Opp_Parser* parser)
{
	struct Opp_Expr* left = opp_parse_or(parser);
	struct Opp_Expr* right = NULL;
	struct Opp_Expr* node = NULL;

	int operator = parser->lex->t.id;

	while (operator == TQUESTION || operator == TCOLON) // fix and check if correct
	{
		opp_next(parser->lex);
		right = opp_parse_ternary(parser);

		node = opp_new_expr(parser, EXPR_TERNARY);
		node->expr.bin.tok = operator;
		node->expr.bin.left = left;
		node->expr.bin.right = right;

		left = node;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Expr* opp_parse_or(struct Opp_Parser* parser)
{
	struct Opp_Expr* left = opp_parse_and(parser);
	struct Opp_Expr* right = NULL;
	struct Opp_Expr* node = NULL;

	int operator = parser->lex->t.id;

	while (operator == TOR)
	{
		opp_next(parser->lex);
		right = opp_parse_and(parser);

		node = opp_new_expr(parser, EXPR_LOGIC);
		node->expr.bin.tok = TOR;
		node->expr.bin.left = left;
		node->expr.bin.right = right;

		left = node;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Expr* opp_parse_and(struct Opp_Parser* parser)
{
	struct Opp_Expr* left = opp_parse_bit_or(parser);
	struct Opp_Expr* right = NULL;
	struct Opp_Expr* node = NULL;

	int operator = parser->lex->t.id;

	while (operator == TAND)
	{
		opp_next(parser->lex);
		right = opp_parse_bit_or(parser);

		node = opp_new_expr(parser, EXPR_LOGIC);
		node->expr.bin.tok = TAND;
		node->expr.bin.left = left;
		node->expr.bin.right = right;

		left = node;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Expr* opp_parse_bit_or(struct Opp_Parser* parser)
{
	struct Opp_Expr* left = opp_parse_bit_xor(parser);
	struct Opp_Expr* right = NULL;
	struct Opp_Expr* node = NULL;

	int operator = parser->lex->t.id;

	while (operator == TBITOR)
	{
		opp_next(parser->lex);
		right = opp_parse_bit_xor(parser);

		node = opp_new_expr(parser, EXPR_BIT);
		node->expr.bin.tok = TBITOR;
		node->expr.bin.left = left;
		node->expr.bin.right = right;

		left = node;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Expr* opp_parse_bit_xor(struct Opp_Parser* parser)
{
	struct Opp_Expr* left = opp_parse_bit_and(parser);
	struct Opp_Expr* right = NULL;
	struct Opp_Expr* node = NULL;

	int operator = parser->lex->t.id;

	while (operator == TBITXOR)
	{
		opp_next(parser->lex);
		right = opp_parse_bit_and(parser);

		node = opp_new_expr(parser, EXPR_BIT);
		node->expr.bin.tok = TBITXOR;
		node->expr.bin.left = left;
		node->expr.bin.right = right;

		left = node;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Expr* opp_parse_bit_and(struct Opp_Parser* parser)
{
	struct Opp_Expr* left = opp_parse_relation(parser);
	struct Opp_Expr* right = NULL;
	struct Opp_Expr* node = NULL;

	int operator = parser->lex->t.id;

	while (operator == TADDR)
	{
		opp_next(parser->lex);
		right = opp_parse_relation(parser);

		node = opp_new_expr(parser, EXPR_BIT);
		node->expr.bin.tok = TADDR;
		node->expr.bin.left = left;
		node->expr.bin.right = right;

		left = node;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Expr* opp_parse_relation(struct Opp_Parser* parser)
{
	struct Opp_Expr* left = opp_parse_relation2(parser);
	struct Opp_Expr* right = NULL;
	struct Opp_Expr* node = NULL;

	int operator = parser->lex->t.id;

	while (operator == TEQEQ || operator == TNOTEQ)
	{
		opp_next(parser->lex);
		right = opp_parse_relation2(parser);

		node = opp_new_expr(parser, EXPR_LOGIC);
		node->expr.bin.tok = operator;
		node->expr.bin.left = left;
		node->expr.bin.right = right;

		left = node;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Expr* opp_parse_relation2(struct Opp_Parser* parser)
{
	struct Opp_Expr* left = opp_parse_shift(parser);
	struct Opp_Expr* right = NULL;
	struct Opp_Expr* node = NULL;

	int operator = parser->lex->t.id;

	while (operator == TGT || operator == TGE ||
		operator == TLT || operator == TLE)
	{
		opp_next(parser->lex);
		right = opp_parse_shift(parser);

		node = opp_new_expr(parser, EXPR_LOGIC);
		node->expr.bin.tok = operator;
		node->expr.bin.left = left;
		node->expr.bin.right = right;

		left = node;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Expr* opp_parse_shift(struct Opp_Parser* parser)
{
	struct Opp_Expr* left = opp_parse_op(parser);
	struct Opp_Expr* right = NULL;
	struct Opp_Expr* node = NULL;

	int operator = parser->lex->t.id;

	while (operator == TSHR || operator == TSHL)
	{
		opp_next(parser->lex);
		right = opp_parse_op(parser);

		node = opp_new_expr(parser, EXPR_BIT);
		node->expr.bin.tok = operator;
		node->expr.bin.left = left;
		node->expr.bin.right = right;

		left = node;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Expr* opp_parse_op(struct Opp_Parser* parser)
{
	struct Opp_Expr* left = opp_parse_op2(parser);
	struct Opp_Expr* right = NULL;
	struct Opp_Expr* node = NULL;

	int operator = parser->lex->t.id;

	while (operator == TADD || operator == TMIN)
	{
		opp_next(parser->lex);
		right = opp_parse_op2(parser);

		node = opp_new_expr(parser, EXPR_BIN);
		node->expr.bin.tok = operator;
		node->expr.bin.left = left;
		node->expr.bin.right = right;

		left = node;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Expr* opp_parse_op2(struct Opp_Parser* parser)
{
	struct Opp_Expr* left = opp_parse_cast(parser);
	struct Opp_Expr* right = NULL;
	struct Opp_Expr* node = NULL;

	int operator = parser->lex->t.id;

	while (operator == TMUL || operator == TDIV ||
		operator == TMOD)
	{
		opp_next(parser->lex);
		right = opp_parse_cast(parser);

		node = opp_new_expr(parser, EXPR_BIN);
		node->expr.bin.tok = operator;
		node->expr.bin.left = left;
		node->expr.bin.right = right;

		left = node;

		operator = parser->lex->t.id;
	}

	return left;
}

static struct Opp_Expr* opp_parse_cast(struct Opp_Parser* parser)
{
	if (parser->lex->t.id == TOPENP) {
		enum Opp_Token t = opp_peek_tok(parser->lex, 1);

		if (opp_get_bucket(parser->state.scope, parser->lex->peek) || 
			is_valid_def(t))
		{
			struct Opp_Expr* node = opp_new_expr(parser, EXPR_CAST);
			node->expr.cast.type = NULL; // add this
			opp_next(parser->lex);
			opp_next(parser->lex);

			if (parser->lex->t.id != TCLOSEP)
				opp_error(parser->lex, "Expected closing ')' in type cast");

			opp_next(parser->lex);

			node->expr.cast.expr = opp_parse_cast(parser);

			return node;
		}
	}

	return opp_parse_prefix(parser);
}

static struct Opp_Expr* opp_parse_prefix(struct Opp_Parser* parser)
{
	struct Opp_Expr* node = NULL;
	switch (parser->lex->t.id)
	{	
		case TMIN:  case TMUL:
		case TADDR: case TNOT:  
		case TBITNOT: {
			node = opp_new_expr(parser, EXPR_PREFIX);
			node->expr.prefix.tok = parser->lex->t.id;
			opp_next(parser->lex);
			node->expr.prefix.expr = opp_parse_cast(parser);
			return node;
		}

		case TINCR: case TDECR: {
			node = opp_new_expr(parser, EXPR_PREFIX);
			node->expr.prefix.tok = parser->lex->t.id;
			opp_next(parser->lex);
			node->expr.prefix.expr = opp_parse_prefix(parser);
			return node;
		}

		case TSIZEOF: { // ( type ) ((expr)) or ( expr )
			break;
		}

		case TADD: {
			opp_next(parser->lex);
			return opp_parse_cast(parser);
		}

		default: break;
	}
	return opp_parse_postfix(parser);
}

static struct Opp_Expr* opp_parse_postfix(struct Opp_Parser* parser)
{
	struct Opp_Expr* expr = opp_parse_unary(parser);

	opp_next(parser->lex);

	int operator = parser->lex->t.id;

	while (operator == TOPENP || operator == TOPENB ||
		operator == TDOT || operator == TDECR || 
		operator == TINCR || operator == TARROW)
	{
		assert(false);
		operator = parser->lex->t.id;
	}

	return expr;
}

static struct Opp_Expr* opp_parse_paran(struct Opp_Parser* parser)
{
	struct Opp_Expr* expr = NULL;

	opp_next(parser->lex);

	expr = opp_parse_comma(parser);

	if (parser->lex->t.id != TCLOSEP)
		opp_error(parser->lex, "Expected closing ')'");

	return expr;
}

static struct Opp_Expr* opp_parse_unary(struct Opp_Parser* parser)
{
	struct Opp_Expr* node = NULL;

	if (parser->lex->t.id == TOPENP)
		return opp_parse_paran(parser);

	node = opp_new_expr(parser, EXPR_UNARY);
	node->expr.unary.type = parser->lex->t.id;

	switch (parser->lex->t.id)
	{
		case TIDENT:
		case TSTR:
			node->expr.unary.val.strval = cpy_string(parser->lex->t.buffer.buf);
			break;

		case TINTEGER:
			node->expr.unary.val.i64val = parser->lex->t.value.num;
			break;

		case TFLOAT:
			node->expr.unary.val.f64val = parser->lex->t.value.real;
			break;

		default:
			opp_error(parser->lex, "Expected a unary value but got '%s'", tok_to_str(parser->lex));
	}

	return node;
}