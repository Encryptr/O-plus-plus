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
#include <assert.h>
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
	struct Opp_Namespace* scope;
};

extern struct Opp_Type_Entry* int_type;
extern struct Opp_Type_Entry* char_type;
extern struct Opp_Type_Entry* float_type; 
extern struct Opp_Type_Entry* void_type; 

struct Opp_Parser* opp_parser_init(struct Opp_Scan* s);
void opp_parser_begin(struct Opp_Parser* parser);
void opp_free_parser(struct Opp_Parser* parser);
struct Opp_Type_Entry* get_type(struct Opp_Type_Tree* tree, char* name);
void opp_add_std_types(struct Opp_Parser* parser);

#endif /* OPP_PARSER */