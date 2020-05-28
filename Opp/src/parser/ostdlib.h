#ifndef OPP_STD
#define OPP_STD

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "ast.h"
#include "../interpreter/interpreter.h"

/* All STDLIB O++ Functions */

#define OPP_MODULE void
#define OPP_ARGS struct Opp_List* args
#define OPP_RETURN struct Opp_Obj* obj
#define LOCAL static 
#define Arg_Val struct Opp_Obj

void opp_init_std();

// echo 
// Basic echo back func
void echo(struct Opp_List* args, struct Opp_Obj* obj);

// print
// Basic print function
void opp_print(struct Opp_List* args, struct Opp_Obj* obj);

// input
// Basic stdin input function
void opp_input(struct Opp_List* args, struct Opp_Obj* obj);

// rand
// Basic random number
void opp_rand(struct Opp_List* args, struct Opp_Obj* obj);

#define expect_args(expected) { \
	if (expected != args->size) \
		opp_error(NULL, "Expected '%d' arguments in function '%s'", expected, __FUNCTION__); \
}

#define type_check(expected, given) { \
	for (int i = 0; i < 10; i++) { \
		if (expected[i] == OBJ_NONE) break; \
		struct Opp_Obj* type = opp_eval_expr(given->list[i]); \
		if (expected[i] != type) { \
			switch (expected[i]) { \
			case OBJ_INT: opp_error(NULL, "Expected argument #%d of type int in func '%s'", i+1, __FUNCTION__);	break; \
			case OBJ_FLOAT:	opp_error(NULL, "Expected argument #%d of type real in func '%s'", i+1, __FUNCTION__);break; \
			case OBJ_BOOL:opp_error(NULL, "Expected argument #%d of type boolean in func '%s'", i+1, __FUNCTION__);break; \
			case OBJ_STR:opp_error(NULL, "Expected argument #%d of type string in func '%s'", i+1, __FUNCTION__);break; \
			} \
		} \

#endif
