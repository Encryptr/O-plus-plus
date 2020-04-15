#ifndef OPP_INTER
#define OPP_INTER

#include <stdio.h>
#include <stdlib.h>
#include "../opp.h"
#include "../env/enviroment.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../parser/ast.h"

enum Opp_Obj_Type {
	OBJ_BOOL, OBJ_INT, OBJ_FLOAT,
	OBJ_STR, OBJ_ARRAY,
};

struct Opp_Obj {
	enum Opp_Obj_Type obj_type;

	union {
		_Bool obool;
		int oint;
		double ofloat;
		char* ostr;
	};
};

struct Opp_Obj* obj_make(enum Opp_Obj_Type obj_type);

void opp_eval_init(struct Opp_Parser* parser);
struct Opp_Obj* opp_eval(struct Opp_Stmt* stmt);
struct Opp_Obj* opp_eval_expr(struct Opp_Expr* expr);
struct Opp_Obj* opp_eval_bin(struct Opp_Expr_Bin* expr);
struct Opp_Obj* opp_eval_logic(struct Opp_Expr_Logic* expr);
struct Opp_Obj* opp_eval_call(struct Opp_Expr_Call* expr);



#endif