#ifndef OPP_STD
#define OPP_STD

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "ast.h"
#include "../interpreter/interpreter.h"

/* All STDLIB O++ Functions */

void opp_init_std();

// echo 
// Basic echo back func
void echo(struct Opp_List* args);

// opp_print
// Basic print function
void opp_print(struct Opp_List* args);

#endif
