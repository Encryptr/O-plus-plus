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
	OBJ_NONE, OBJ_BOOL, OBJ_INT, 
	OBJ_FLOAT, OBJ_STR, OBJ_ARRAY,
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
void opp_repl_ret(struct Opp_Obj* val);

struct Opp_Obj* opp_eval(struct Opp_Stmt* stmt);
struct Opp_Obj* opp_eval_expr(struct Opp_Expr* expr);
struct Opp_Obj* opp_eval_bin(struct Opp_Expr_Bin* expr);
struct Opp_Obj* opp_eval_logic(struct Opp_Expr_Logic* expr);
struct Opp_Obj* opp_eval_call(struct Opp_Expr_Call* expr);
struct Opp_Obj* opp_eval_ifstmt(struct Opp_Stmt_If* expr);
struct Opp_Obj* opp_eval_block(struct Opp_Stmt_Block* expr);
struct Opp_Obj* opp_eval_assign(struct Opp_Expr_Assign* expr);
struct Opp_Obj* opp_eval_var(struct Opp_Stmt_Var* expr);
struct Opp_Obj* opp_eval_import(struct Opp_Stmt_Import* expr);
struct Opp_Obj* opp_eval_while(struct Opp_Stmt_While* expr);

#endif