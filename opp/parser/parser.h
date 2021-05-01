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

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

enum Scope_State {
	SCOPE_GLOBAL,
	SCOPE_PARAM,
	SCOPE_LOCAL,
};

struct Opp_Parser_State {
	enum Scope_State scope;
	char* decl_name;
};

struct Opp_Parser {
	struct Opp_Scan* lex;
    struct Opp_Stmt** statements;
	size_t allocated, nstmts;
	struct Opp_Parser_State state;
	struct Opp_Hashmap* type_map;
};

struct Opp_Parser* opp_init_parser(struct Opp_Scan* s);
void opp_parser_begin(struct Opp_Parser* parser);

#endif /* OPP_PARSER */
