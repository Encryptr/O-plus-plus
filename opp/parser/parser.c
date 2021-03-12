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

static struct Opp_Stmt* opp_new_stmt(struct Opp_Parser* parser, enum Opp_Stmt_Type type);
static struct Opp_Expr* opp_new_expr(struct Opp_Parser* parser, enum Opp_Expr_Type type);
static struct Opp_Type opp_parse_type(struct Opp_Parser* parser);

static struct Opp_Stmt* opp_parse_global_def(struct Opp_Parser* parser);
static struct Opp_Stmt* opp_parse_declaration(struct Opp_Parser* parser);

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

		// handel null rule

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

static struct Opp_Stmt* opp_parse_global_def(struct Opp_Parser* parser)
{
	parser->state.scope = SCOPE_GLOBAL;
	switch (parser->lex->t.id)
	{
		case TIDENT:    case TEXTERN:  case TSTATIC: 
		case TINLINE:   case TTYPEDEF: case TUNION:  
		case TENUM:     case TSTRUCT:  case TCHAR:   
		case TSHORT:    case TINT:     case TLONG:    
		case TUNSIGNED: case TSIGNED:  case TCONST:   
		case TVOLATILE: case TFLOAT:   case TDOUBLE:
		case TVOID:     case TREGITSTER: case TAUTO:
			return opp_parse_declaration(parser);
	
		default:
			opp_error(parser->lex, "Error unexpected global declaration '%s'", 
				tok_to_str(parser->lex));
	}
	return NULL;
}

static struct Opp_Type opp_parse_type(struct Opp_Parser* parser)
{
	struct Opp_Type type = {0};

	while (parser->lex->t.id != FEND) {
		switch (parser->lex->t.id) {
			case TEXTERN:
			case TSTATIC: 
			case TAUTO:   
			case TREGITSTER: 
			case TTYPEDEF: {
				if (type.storage_class != 0)
					opp_error(parser->lex, 
						"Another storage class detected in declaration '%s'", 
						parser->lex->t.buffer.buf);

				if (parser->state.scope == SCOPE_GLOBAL && 
					(parser->lex->t.id == TREGITSTER || parser->lex->t.id == TAUTO))
					opp_error(parser->lex, "Unexpected '%s' in global scope", 
						parser->lex->t.buffer.buf);
				type.storage_class = parser->lex->t.id;
				break;
			}

			case TSTRUCT: 
		    case TENUM:
			case TUNION:
			break;

		    case TCHAR: 
		    case TSHORT: 
		    case TINT: 
		    case TLONG: 
		    case TFLOAT: 
		    case TDOUBLE: 
			case TVOID: {
				switch (type.type) {
					case TINT: {
						if (parser->lex->t.id != TSHORT && 
							parser->lex->t.id != TLONG)
							opp_error(parser->lex, "Another type found in declaration '%s'", 
								parser->lex->t.buffer.buf);
						type.type = parser->lex->t.id;
						break;
					}

					case TSHORT: case TLONG: {
						if (parser->lex->t.id != TINT)
							opp_error(parser->lex, "Another type found in declaration '%s'", 
								parser->lex->t.buffer.buf);
						break;
					}

					default: {
						if (type.type != 0)
							opp_error(parser->lex, "Another type found in declaration '%s'", 
								parser->lex->t.buffer.buf);
						type.type = parser->lex->t.id;
						break;
					}
				}
				break;
			}

			case TINLINE:
			case TRESTRICT:
			case TVOLATILE:
			case TCONST: {
				// if (type.attribute != 0 && type.attribute == type.a)
				// 	opp_error(parser->lex, "Another type qualifier found '%s'", 
				// 		parser->lex->t.buffer.buf);
				// type.attribute = parser->lex->t.id;
				break;
			}

			case TUNSIGNED:
			case TSIGNED: {
				if (type.sign != 0)
					opp_error(parser->lex, "Unexpected '%s' type attribute",
						parser->lex->t.buffer.buf);
				type.sign = parser->lex->t.id == TSIGNED ? 1 : 2;
				break; 
			}

			case TIDENT: {
				printf("CHECK TYPEDEF\n");
				break;
			}

			default:
				break;
		}
		opp_next(parser->lex);
	}

	if (type.storage_class == 0)
		type.storage_class = TEXTERN;
	if (type.type == 0)
		type.type = TINT;
	if (type.sign == 0)
		type.sign = 1;
	
	return type;
}

static struct Opp_Stmt* opp_parse_declaration(struct Opp_Parser* parser)
{
	struct Opp_Stmt* stmt = NULL;

	struct Opp_Type t = opp_parse_type(parser);

	printf("===>%s\n", tok_debug(t.type));
	printf("===>%s\n", tok_debug(t.storage_class));
	printf("===>%s\n", t.sign == 1 ? "signed" : "unsigned");

	return stmt;
}