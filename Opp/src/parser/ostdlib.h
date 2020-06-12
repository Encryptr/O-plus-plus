#ifndef OPP_STD
#define OPP_STD

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "ast.h"
#include "../env/enviroment.h"
#include "../interpreter/interpreter.h"

/* All STDLIB O++ Functions */

#define OPP_MODULE void
#define OPP_ARGS struct Opp_List* args
#define OPP_RETURN struct Opp_Obj* obj
#define OPP_ENV struct Namespace* env
#define LOCAL static 
#define Arg_Val struct Opp_Obj
#define ADD_FUNC(a,b) { if (!env_new_cfn(env->inside, a, b)) internal_error("init fail", 2); }

// Return types
#define RETURN_NONE obj->obj_type = OBJ_NONE
#define RETURN_NUM obj->obj_type = OBJ_INT
#define RETURN_BOOL obj->obj_type = OBJ_BOOL
#define RETURN_REAL obj->obj_type = OBJ_FLOAT
#define RETURN_STR obj->obj_type = OBJ_STR

#define Return_Value(a) switch (obj->obj_type) { \
	case OBJ_NONE: break; \
	case OBJ_INT: obj->oint = a; break; \
	case OBJ_BOOL: obj->obool = a; break; \
	case OBJ_FLOAT: obj->ofloat = a; break; \
	case OBJ_STR: strcpy(obj->ostr, a); break; \
	case OBJ_ARRAY: break; \
}


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

// typeof
// Basic function returning identifier type
void opp_typeof(struct Opp_List* args, struct Opp_Obj* obj);

// append
// A function to append to an array
void opp_append(struct Opp_List* args, struct Opp_Obj* obj);

// getc
// Get single char
void opp_getc(struct Opp_List* args, struct Opp_Obj* obj);

// str_to_num
// Convert string to number
void opp_strtonum(struct Opp_List* args, struct Opp_Obj* obj);

// len
// Get length of str / array
void opp_len(struct Opp_List* args, struct Opp_Obj* obj);

#define expect_args(expected) { \
	if (expected != args->size) \
		opp_error(NULL, "Expected '%d' arguments in function '%s'", expected, __FUNCTION__); \
}

#define type_check(expected, given) { \
	for (int i = 0; i < sizeof(expected)/sizeof(expected[0]); i++) \
	{ \
		struct Opp_Obj type; opp_eval_expr(given->list[i], &type); \
		if (expected[i] != type.obj_type) { \
			switch (expected[i]) { \
			case OBJ_INT: opp_error(NULL, "Expected argument #%d of type int in func '%s'", i+1, __FUNCTION__);	break; \
			case OBJ_FLOAT:	opp_error(NULL, "Expected argument #%d of type real in func '%s'", i+1, __FUNCTION__);break; \
			case OBJ_BOOL:opp_error(NULL, "Expected argument #%d of type boolean in func '%s'", i+1, __FUNCTION__);break; \
			case OBJ_STR:opp_error(NULL, "Expected argument #%d of type string in func '%s'", i+1, __FUNCTION__);break; \
			} \
		} \
	}}

#endif
