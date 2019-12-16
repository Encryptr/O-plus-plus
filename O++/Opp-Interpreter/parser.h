#ifndef OPP_PARSER
#define OPP_PARSER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "ast.h"
#include "hashmap.h"

/*
struct Opp_Node* opp_parser_init(struct Scan* s);
struct Opp_Node* opp_parser(struct Scan* s);

struct Opp_Node* opp_parse_fend(struct Scan* s);
struct Opp_Node* opp_parse_num(struct Scan* s);
struct Opp_Node* opp_parse_nil(struct Scan* s);

struct Opp_Node* opp_parse_stmt_start(struct Scan* s);
*/

void expect_semi(struct Scan* s);

void opp_init_parser(struct Scan* s);
void opp_parser(struct Scan* s);

struct Opp_Value opp_parse_num(struct Scan* s);
struct Opp_Value opp_parse_varstr(struct Scan* s);
struct Opp_Value opp_expr_one(struct Scan* s);
struct Opp_Value opp_expr_two(struct Scan* s);
struct Opp_Value opp_parse_str(struct Scan* s);
struct Opp_Value opp_parse_type(struct Scan* s);

void opp_init_stdlib();
void opp_std_print(struct Scan* s);

void opp_parse_var(struct Scan* s);

#endif