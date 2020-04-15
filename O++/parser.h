#ifndef OPP_PARSER
#define OPP_PARSER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "object.h"
#include "hashmap.h"

void expect_semi(struct Scan* s);

void opp_init_parser(struct Scan* s);
void opp_declaration(struct Scan* s);
void opp_statment(struct Scan* s);
void opp_expression(struct Scan* s);

// STD LIB
void opp_std_print(struct Scan* s);

#endif