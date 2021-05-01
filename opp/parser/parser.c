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
static struct Opp_Type* opp_parse_declarator(struct Opp_Parser* parser, 
											 struct Opp_Type* base);
static struct Opp_Type* opp_parse_post_decl(struct Opp_Parser* parser, struct Opp_Type* base);

// Statements
static struct Opp_Stmt* opp_parse_stmt(struct Opp_Parser* parser);
static struct Opp_Stmt* opp_parse_definition(struct Opp_Parser* parser, struct Opp_Type* type);
static struct Opp_Stmt* opp_decl(struct Opp_Parser* parser, struct Opp_Type* type, bool init);
static struct Opp_Stmt* opp_func_decl(struct Opp_Parser* parser, struct Opp_Type* type);

// Expressions
static struct Opp_Expr* opp_parse_expr(struct Opp_Parser* parser);
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

static bool is_valid_def(struct Opp_Parser* parser)
{
	switch (parser->lex->t.id) {
		case TIDENT:     case TEXTERN:  case TSTATIC: 
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
	parser->state.scope = SCOPE_GLOBAL;
	switch (parser->lex->t.id)
	{
		case TIDENT:     case TEXTERN:  case TSTATIC: 
		case TINLINE:    case TUNION:   case TENUM:     
		case TSTRUCT:    case TCHAR:    case TSHORT:
		case TINT:       case TLONG:    case TUNSIGNED:
		case TSIGNED:    case TCONST:   case TVOLATILE:
		case TFLOAT:     case TDOUBLE:  case TVOID:
		case TREGITSTER: case TAUTO:
			return opp_parse_decl(parser);

		case TTYPEDEF:
		break;
	
		default:
			opp_error(parser->lex, "Error unexpected global declaration '%s'", 
				tok_to_str(parser->lex));
	}
	return NULL;
}

#define ANOTHER() \
	opp_error(parser->lex, "Another attribute found in declaration '%s'", parser->lex->t.buffer.buf)

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
			case TAUTO:   case TREGITSTER: {
				if (type->storage_class != 0)
					ANOTHER();
				if (parser->state.scope == SCOPE_GLOBAL && 
					(parser->lex->t.id == TREGITSTER || parser->lex->t.id == TAUTO))
						opp_error(parser->lex, "Unexpected '%s' in global scope", 
							parser->lex->t.buffer.buf);
				type->storage_class = parser->lex->t.id;
				break;
			}

			case TSTRUCT:
			case TUNION:
			break;

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
				goto end;
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

static struct Opp_Type* opp_parse_param(struct Opp_Parser* parser, struct Opp_Type* base)
{
	char* cpy = parser->state.decl_name;
	base = opp_create_type(TYPE_FUNC, base);
	base->val.fn.param = (struct Opp_Func_Mem*)opp_alloc(sizeof(struct Opp_Func_Mem)*6);
	if (!base->val.fn.param)
		MALLOC_FAIL();
	unsigned int len = 0;

	opp_next(parser->lex);
	while (parser->lex->t.id != TCLOSEP) {
		if (len == 6)
			opp_error(parser->lex, "Max func param length met (6)");
		if (parser->lex->t.id == TVA_ARGS) {
			base->val.fn.is_vaarg = true;
			opp_next(parser->lex);
			break;
		}

		if (!is_valid_def(parser))
			opp_error(parser->lex, "Invalid definition in func paramter #%d", len);

		struct Opp_Type* type = opp_parse_declaration_specifier(parser);

		if (type->storage_class == TEXTERN || type->storage_class == TSTATIC)
			opp_error(parser->lex, "Unexpected storage class in function paramater");

		if (type->type == TYPE_NONE)
			type->type = TYPE_INT;

		type = opp_parse_declarator(parser, type);

		base->val.fn.param[len].name = parser->state.decl_name;
		base->val.fn.param[len].type = type;

		if (parser->lex->t.id == TCOMMA)
			opp_next(parser->lex);
		len++;
	}

	if (parser->lex->t.id != TCLOSEP)
		opp_error(parser->lex, "Expected ')' in function declaration");

	base->val.fn.len = len;
	parser->state.decl_name = cpy;

	return base;
}

static struct Opp_Type* opp_parse_post_decl(struct Opp_Parser* parser, struct Opp_Type* base)
{
	struct Opp_Type* nest = NULL;
	if (parser->lex->t.id == TOPENP) { // check if not func
		opp_next(parser->lex);
		nest = opp_parse_declarator(parser, nest);

		if (parser->lex->t.id != TCLOSEP)
			opp_error(parser->lex, "Expected closing ')'");
		opp_next(parser->lex);
	}
	else if (parser->lex->t.id == TIDENT) {
		parser->state.decl_name = opp_alloc(strlen(parser->lex->t.buffer.buf) + 1);
		if (!parser->state.decl_name)
			MALLOC_FAIL();
		strcpy(parser->state.decl_name, parser->lex->t.buffer.buf);
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

static struct Opp_Type* opp_parse_pointer(struct Opp_Parser* parser, struct Opp_Type* base)
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

	base = opp_parse_post_decl(parser, base);

	return base;
}

static struct Opp_Type* opp_parse_declarator(struct Opp_Parser* parser, struct Opp_Type* base)
{
	return opp_parse_pointer(parser, base);
}

static struct Opp_Stmt* opp_parse_definition(struct Opp_Parser* parser, struct Opp_Type* type)
{
	struct Opp_Stmt* stmt = NULL;
	if (parser->lex->t.id == TOPENC) 
		stmt = opp_func_decl(parser, type);
	else 
		stmt = opp_decl(parser, type, parser->lex->t.id == TEQ);

	return stmt;
}

static struct Opp_Stmt* opp_decl(struct Opp_Parser* parser, struct Opp_Type* type, bool init)
{
	struct Opp_Stmt* decl = opp_new_stmt(parser, STMT_DECL);

	assert(!init);

	decl->stmt.decl.name = parser->state.decl_name;
	decl->stmt.decl.type = type;

	return decl;
}

static struct Opp_Stmt* opp_func_decl(struct Opp_Parser* parser, struct Opp_Type* type)
{
	struct Opp_Stmt* func = opp_new_stmt(parser, STMT_FUNC);

	func->stmt.func.name = parser->state.decl_name;
	func->stmt.func.type = type;
	// func->stmt.func.body //parse

	return func;
}

static struct Opp_Stmt* opp_parse_decl(struct Opp_Parser* parser)
{
	parser->state.decl_name = NULL;
	struct Opp_Type* base = opp_parse_declaration_specifier(parser);
	struct Opp_Type* t = opp_parse_declarator(parser, base);

	struct Opp_Stmt* stmt = opp_parse_definition(parser, t);

	if (stmt->type == STMT_FUNC)
		return stmt;

	struct Opp_Stmt* list = stmt;
	while (parser->lex->t.id == TCOMMA) {
		opp_next(parser->lex);
		t = opp_parse_declarator(parser, base);
		list->stmt.decl.next = opp_parse_definition(parser, t);
		list = list->stmt.decl.next;
	}

	if (parser->lex->t.id != TSEMICOLON)
		opp_error(parser->lex, "Expected ';' after declaration");

	opp_debug_type(t);

	return stmt;
}

static struct Opp_Stmt* opp_parse_stmt(struct Opp_Parser* parser)
{
	switch (parser->lex->t.id)
	{
		case TIF:
		case TWHILE:
		case TDO:
		case TFOR:
		case TSWITCH:
		case TCASE:
		case TDEFAULT:
		case TRETURN:
		case TCONTINUE:
		case TSEMICOLON:
		case TOPENB:
		break;
		default:
			break;
	}
	return NULL;
}

static struct Opp_Expr* opp_parse_expr(struct Opp_Parser* parser)
{
	struct Opp_Expr* expr = opp_parse_assign(parser);

	if (parser->lex->t.id != TSEMICOLON)
		opp_error(parser->lex, "Expected ';' after expression");

	return expr;
}

static struct Opp_Expr* opp_parse_assign(struct Opp_Parser* parser)
{
	struct Opp_Expr* left = opp_parse_ternary(parser);
	struct Opp_Expr* right = NULL;
	struct Opp_Expr* node = NULL;

	while (parser->lex->t.id == TEQ || parser->lex->t.id == TADDEQ ||
		parser->lex->t.id == TMINEQ || parser->lex->t.id == TDIVEQ ||
		parser->lex->t.id == TMULEQ || parser->lex->t.id == TMODEQ ||
		parser->lex->t.id == TSHLEQ || parser->lex->t.id == TSHREQ ||
		parser->lex->t.id == TANDEQ || parser->lex->t.id == TOREQ)
	{
		opp_next(parser->lex);

		right = opp_parse_ternary(parser);

		node = opp_new_expr(parser, EXPR_ASSIGN);
		node->expr.bin.tok = parser->lex->t.id;
		node->expr.bin.left = left;
		node->expr.bin.right = right;

		left = node;
	}
}

static struct Opp_Expr* opp_parse_ternary(struct Opp_Parser* parser)
{

}

static struct Opp_Expr* opp_parse_or(struct Opp_Parser* parser)
{

}

static struct Opp_Expr* opp_parse_and(struct Opp_Parser* parser)
{

}

static struct Opp_Expr* opp_parse_bit_or(struct Opp_Parser* parser)
{

}

static struct Opp_Expr* opp_parse_bit_xor(struct Opp_Parser* parser)
{

}

static struct Opp_Expr* opp_parse_bit_and(struct Opp_Parser* parser)
{

}

static struct Opp_Expr* opp_parse_relation(struct Opp_Parser* parser)
{

}

static struct Opp_Expr* opp_parse_relation2(struct Opp_Parser* parser)
{

}

static struct Opp_Expr* opp_parse_shift(struct Opp_Parser* parser)
{

}

static struct Opp_Expr* opp_parse_op(struct Opp_Parser* parser)
{

}

static struct Opp_Expr* opp_parse_op2(struct Opp_Parser* parser)
{

}

static struct Opp_Expr* opp_parse_prefix(struct Opp_Parser* parser)
{

}

static struct Opp_Expr* opp_parse_postfix(struct Opp_Parser* parser)
{

}

static struct Opp_Expr* opp_parse_unary(struct Opp_Parser* parser)
{
	(void)parser;
	return NULL;
}