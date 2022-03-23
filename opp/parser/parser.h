/** @file parser.h
 * 
 * @brief Opp Parser
 *      
 * Copyright (c) 2022 Maks S
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
**/

#ifndef OPP_PARSER
#define OPP_PARSER

#include "../vm/vm.h"
#include "../util/array.h"
#include "../util/platform.h"

enum Scope {
    SCOPE_GLOBAL,
    SCOPE_FUNC
};

struct Local_Var {
    char ident[MAX_IDENTIFIER_LEN];
    unsigned int depth;
};

struct Func_Ctx {
    struct Local_Var locals;
    unsigned int local_count;
    struct Opp_Func_Obj* fn;
};

struct Opp_Parser {
    struct Opp_Scan* lex;
    enum Scope scope;
    struct Hashmap* globals;
    struct Func_Ctx* func;
};

struct Opp_Parser* opp_init_parser(struct Opp_Scan* const s);
void opp_free_parser(struct Opp_Parser* const parser);
void opp_begin_parser(struct Opp_Parser* const parser);

#endif /* OPP_PARSER */