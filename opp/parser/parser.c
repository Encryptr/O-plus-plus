/** @file parser.c
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

#include "parser.h"
#include "../memory/memory.h"
#include "../util/util.h"
#include "../lexer/lexer.h"

#define TOKEN_TYPE(parser) \
    parser->lex->t.id

static void opp_parse_stmt(struct Opp_Parser* const parser);
static void opp_parse_ident(struct Opp_Parser* const parser);
static void opp_parse_func(struct Opp_Parser* const parser, char* const ident);
static void opp_parse_var(struct Opp_Parser* const parser, char* const ident);
static void opp_parse_expr(struct Opp_Parser* const parser);

// static struct Local_Var* opp_init_locals()
// {
//     struct Local_Var* locals = (struct Local_Var*)
//         opp_alloc(sizeof(struct Local_Var) * LOCAL_LIST_SIZE);
    
//     if (!locals)
//         MALLOC_FAIL();

//     memset(locals, 0, sizeof(struct Local_Var) * LOCAL_LIST_SIZE);

//     return locals;
// }

// static struct Func_Ctx* opp_new_func(struct Opp_Parser* const parser)
// {
//     struct Func_Ctx* ctx = (struct Func_Ctx*)
//         opp_alloc(sizeof(struct Func_Ctx));
    
//     if (!ctx)
//         MALLOC_FAIL();

    

// }

struct Opp_Parser* opp_init_parser(struct Opp_Scan* const s)
{
    struct Opp_Parser* parser = (struct Opp_Parser*)
        opp_os_alloc(sizeof(struct Opp_Parser));

    parser->scope = SCOPE_GLOBAL;
    parser->lex = s;

    // parser->globals = opp_create_map(GLOBAL_HASHMAP_SIZE, NULL);

    return parser;
}

void opp_free_parser(struct Opp_Parser* const parser)
{
    opp_os_free(parser);
}

void opp_begin_parser(struct Opp_Parser* const parser)
{
    if (!parser)
        INTERNAL_ERROR("Parser object is null");

    for (;;) {
        opp_next(parser->lex);

        if (parser->lex->t.id == FEND)
            break;
        
        opp_parse_stmt(parser);
    }
}

static void opp_parse_stmt(struct Opp_Parser* const parser)
{
    switch (parser->lex->t.id)
    {
        /* <indent> ( <args> ) | <ident> = <expr>*/
        case TIDENT: 
            opp_parse_ident(parser);
            break;

        default:
            opp_error(parser->lex, "Expected a statement");

    }
}

static void opp_parse_ident(struct Opp_Parser* const parser)
{
    char* const ident = "";
    opp_next(parser->lex);

    if (TOKEN_TYPE(parser) == TEQ)
        opp_parse_var(parser, ident);
    else if (TOKEN_TYPE(parser) == TOPENP)
        opp_parse_func(parser, ident);
    else
        opp_error(parser->lex, "Expected '=' or '(' after identifier '%s'", ident);
}

static void opp_parse_var(struct Opp_Parser* const parser, char* const ident)
{
    opp_next(parser->lex);

    opp_parse_expr(parser);

    // if (parser->scope == SCOPE_GLOBAL)

}

static void opp_parse_func(struct Opp_Parser* const parser, char* const ident)
{

}

static void opp_parse_expr(struct Opp_Parser* const parser)
{

}