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

/* Operator precedence (lowest-highest)
    or 
    and
    |
    ^
    &
    == !=
    > < >= <=
    >> <<
    + - 
    * / %

    Prefix: 
    - ! ~

    Postfix:
    () [] . ++ --
*/

typedef enum {
    P_NONE,
    P_OR,
    P_AND,
    P_BITOR,
    P_BITXOR,
    P_BITAND,
    P_EQUALITY,
    P_COMAPRISON,
    P_SHIFT,
    P_TERM,
    P_FACTOR,
    P_UNARY,
} Precedence;

#define ENTRY(id) [id - TK_CHARS]
Precedence const bin_prec_table[] = {
    ENTRY(TOR)     = P_OR,
    ENTRY(TAND)    = P_AND,
    ENTRY(TBITOR)  = P_BITOR,
    ENTRY(TBITXOR) = P_BITXOR,
    ENTRY(TEQEQ)   = P_EQUALITY, 
    ENTRY(TNOTEQ)  = P_EQUALITY,
    ENTRY(TLT)     = P_COMAPRISON,
    ENTRY(TGT)     = P_COMAPRISON,
    ENTRY(TLE)     = P_COMAPRISON,
    ENTRY(TGE)     = P_COMAPRISON,
    ENTRY(TSHL)    = P_SHIFT,
    ENTRY(TSHR)    = P_SHIFT,
    ENTRY(TADD)    = P_TERM,
    ENTRY(TMIN)    = P_TERM,
    ENTRY(TMUL)    = P_FACTOR,
    ENTRY(TDIV)    = P_FACTOR,
    ENTRY(TMOD)    = P_FACTOR,
};

#define UNARY_OP(tok) \
    (tok == TMIN || tok == TNOT || tok == TBITNOT)

#define BINARY_OP(tok) \
    ((tok > TEQ && tok <= TADDR) || tok == TAND || tok == TOR) ? tok : INVALID

#define GET_BIN_PREC(tok) \
    bin_prec_table[tok - TK_CHARS]

#define next(parser) \
    opp_next(parser->lex)

#define peek(parser, n) \
    opp_peek_tok(parser->lex, n)

static void opp_parse_stmt(struct Opp_Parser* const parser);
static void opp_parse_ident(struct Opp_Parser* const parser);
static void opp_parse_func(struct Opp_Parser* const parser);
static void opp_parse_assigment(struct Opp_Parser* const parser);

static void opp_parse_expr_stmt(struct Opp_Parser* const parser);
static void opp_parse_sub_expr(struct Opp_Parser* const parser, Precedence prec);
static void opp_parse_unary(struct Opp_Parser* const parser);
static void opp_parse_prefix(struct Opp_Parser* const parser);

static void opp_init_func_scope(struct Opp_Parser* const parser)
{
    parser->func_scope = (struct Func_Ctx*)
        opp_os_alloc(sizeof(struct Func_Ctx) * MAX_FUNC_DEPTH);

    MALLOC_FAIL(!parser->func_scope);

    parser->func_top = parser->func_scope;
}

struct Opp_Parser* opp_init_parser(struct Opp_Scan* const s)
{
    struct Opp_Parser* parser = (struct Opp_Parser*)
        opp_os_alloc(sizeof(struct Opp_Parser));

    MALLOC_FAIL(!parser);

    parser->scope = SCOPE_GLOBAL;
    parser->lex = s;
    parser->globals = init_map(GLOBAL_HASHMAP_SIZE, NULL);
    opp_init_func_scope(parser);

    return parser;
}

void opp_free_parser(struct Opp_Parser* const parser)
{
    free_map(parser->globals);
    opp_os_free(parser->func_scope);
    opp_os_free(parser);
}

void opp_begin_parser(struct Opp_Parser* const parser)
{
    OPP_ASSERT(parser);

    for (;;) {
        next(parser);
        if (TOKEN_TYPE(parser) == FEND) 
            break;
        opp_parse_stmt(parser);
    }
}

static bool opp_check_func_decl(struct Opp_Parser* const parser)
{
    int paran_amount = 1;
    unsigned int i = 2;

    while (paran_amount > 0) {
        const enum Opp_Token t = opp_peek_tok(parser->lex, i);
        if (t == TOPENP) 
            ++paran_amount;
        else if (t == TCLOSEP) 
            --paran_amount;
        else if (t == FEND)
            opp_error(parser->lex, "End of file in declaration, no terminating ')'");
        i++;
    }

    return opp_peek_tok(parser->lex, i) == TOPENC;
}

static void opp_parse_stmt(struct Opp_Parser* const parser)
{
    switch (parser->lex->t.id)
    {
        /* <indent> ( <args> ) 
            | <ident> = <expr>
            | <ident>++
            | <ident>--
        */
        case TIDENT: {
            enum Opp_Token t = opp_peek_tok(parser->lex, 1);

            if (t == TEQ) {
                opp_parse_assigment(parser);
                break;
            }
            else if (t == TOPENP && opp_check_func_decl(parser)) {
                opp_parse_func(parser);
                break;
            }
            // Fall through since it must be an expr
        }

        default: {
            opp_parse_expr_stmt(parser);
            break;
        }
    }
}

static void opp_parse_expr_stmt(struct Opp_Parser* const parser)
{
    opp_parse_sub_expr(parser, P_NONE);
    printf("Pop\n");
}

static void opp_parse_assigment(struct Opp_Parser* const parser)
{
}

static void opp_parse_func(struct Opp_Parser* const parser)
{
}

static void opp_parse_sub_expr(struct Opp_Parser* const parser, Precedence prec)
{
    if (UNARY_OP(TOKEN_TYPE(parser)))
        opp_parse_prefix(parser);
    else
        opp_parse_unary(parser);

    enum Opp_Token op = BINARY_OP(opp_peek_tok(parser->lex, 1));
    
    while (op && prec < GET_BIN_PREC(op)) {
        next(parser);
        next(parser);
        
        opp_parse_sub_expr(parser, GET_BIN_PREC(op));
        
        printf("%s\n", tok_debug(op));

        op = BINARY_OP(opp_peek_tok(parser->lex, 1));
    }
}

static void opp_parse_binary(struct Opp_Parser* const parser)
{

}

static void opp_parse_prefix(struct Opp_Parser* const parser)
{
    const enum Opp_Token op = TOKEN_TYPE(parser);

    next(parser);

    // Check if const so then we can optimize

    opp_parse_sub_expr(parser, P_UNARY);

    printf("%s\n", tok_debug(op));
}

static void opp_parse_unary(struct Opp_Parser* const parser)
{
    switch (TOKEN_TYPE(parser)) 
    {
        case TOPENP: {
            next(parser);
            opp_parse_sub_expr(parser, P_OR);
            next(parser);
            if (TOKEN_TYPE(parser) != TCLOSEP)
                opp_error(parser->lex, "Expected terminating ')'");
            break;
        }

        case TIDENT: 
            printf("Push %s\n", parser->lex->t.buffer);
            break;

        case TFLOATING: break;
        case TINTEGER: 
            printf("Push %lld\n", parser->lex->t.value.num);
            break;
        case TSTR: break;
        default:
            opp_error(parser->lex, "Expected a value, got '%s'", 
                tok_debug(TOKEN_TYPE(parser)));
    }

    // Parse postfix

    for (;;) {
        const enum Opp_Token tok = opp_peek_tok(parser->lex, 1);
        switch (tok)
        {
            case TOPENP:
                next(parser);
                printf("Call\n");
                next(parser);
                break;

            case TOPENB:
                break;

            case TDOT:
                break;
            
            default:
                return;
        }
    }
}