/** @file parser.h
 * 
 * @brief Opp Parser header file
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

#ifndef OPP_PARSER
#define OPP_PARSER

#include "../opp.h"
#include "../lexer/lexer.h"
#include "../ast/ast.h"
#include "types.h"
#include <ctype.h>

#define DEFAULT_TYPE_TREE_SIZE 160
#define DEFAULT_LIST_SIZE 8

struct Opp_Parser {
	struct Opp_Scan* lex;
	struct Opp_Node** statments;
	size_t stmt_size;
	unsigned int nstmts;
	struct Opp_Type_Tree tree;
};

struct Opp_Type_Entry* int_type;
struct Opp_Type_Entry* char_type;
struct Opp_Type_Entry* float_type; 
struct Opp_Type_Entry* void_type; 

struct Opp_Parser* opp_parser_init(struct Opp_Scan* s);
void opp_parser_begin(struct Opp_Parser* parser);

static void opp_debug_parser(struct Opp_Node* node);
static void opp_expect_error(struct Opp_Parser* parser, char sym);

static struct Opp_Node* opp_new_node(struct Opp_Parser* parser, enum Opp_Node_Type type);
static struct Opp_Node* opp_parse_global_def(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_type(struct Opp_Parser* parser);
static struct Opp_List* opp_parse_args(struct Opp_Parser* parser);
static bool opp_parse_peak(struct Opp_Parser* parser, char ch);

// Statements
static struct Opp_Node* opp_parse_statement(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_block(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_var_decl(struct Opp_Parser* parser, struct Opp_Type_Decl type);
static struct Opp_Node* opp_parse_func(struct Opp_Parser* parser, struct Opp_Type_Decl type);
static struct Opp_Node* opp_parse_struct(struct Opp_Parser* parser);

static struct Opp_Node* opp_parse_return(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_while(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_label(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_goto(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_if(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_extern(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_import(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_for(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_switch(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_case(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_break(struct Opp_Parser* parser);

// Expressions
static struct Opp_Node* opp_parse_shifts(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_bit_and(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_bit_or(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_bit_bit_xor(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_bit_bit_not(struct Opp_Parser* parser);

static struct Opp_Node* opp_parse_expr(struct Opp_Parser* parser);
static struct Opp_List* opp_parse_comma(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_allign(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_or(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_and(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_relation(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_comparison(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_order1(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_order2(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_before(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_prefix(struct Opp_Parser* parser);
static struct Opp_Node* opp_parse_unary(struct Opp_Parser* parser);

// Unary
static void opp_parse_identifier(struct Opp_Parser* parser, struct Opp_Node* node);
static void opp_parse_integer(struct Opp_Parser* parser, struct Opp_Node* node);
static void opp_parse_float(struct Opp_Parser* parser, struct Opp_Node* node);
static void opp_parse_str(struct Opp_Parser* parser, struct Opp_Node* node);
static struct Opp_Node* opp_parse_paran(struct Opp_Parser* parser);


#endif /* OPP_PARSER */