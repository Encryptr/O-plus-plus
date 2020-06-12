#ifndef OPP_INTER
#define OPP_INTER
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include "../opp.h"
#include "../env/enviroment.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../parser/ast.h"

#define MAX_OSTR 100

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
		char ostr[MAX_OSTR];
	};
};

struct Opp_Process {
	int trigger_ret;
	struct Opp_Obj val;
};

struct Opp_Array {
	int used;
	struct Opp_Value array;
};

struct Opp_Process opp_state;
struct Opp_Array opp_array;

struct Opp_Obj* obj_make(enum Opp_Obj_Type obj_type);
void opp_eval_init(struct Opp_Parser* parser);
void opp_repl_ret(struct Opp_Obj* val);
void opp_repl_array(struct Opp_Obj* val);

void opp_eval(struct Opp_Stmt* stmt, struct Opp_Obj* obj);
void opp_eval_expr(struct Opp_Expr* expr, struct Opp_Obj* literal);

void opp_eval_bin(struct Opp_Expr_Bin* expr, struct Opp_Obj* obj);
void opp_eval_logic(struct Opp_Expr_Logic* expr, struct Opp_Obj* obj);
void opp_eval_call(struct Opp_Expr_Call* expr, struct Opp_Obj* obj);
void opp_eval_ifstmt(struct Opp_Stmt_If* expr, struct Opp_Obj* obj);
void opp_eval_block(struct Opp_Stmt_Block* expr, struct Opp_Obj* obj);
void opp_eval_assign(struct Opp_Expr_Assign* expr, struct Opp_Obj* obj);
void opp_eval_var(struct Opp_Stmt_Var* expr, struct Opp_Obj* obj);
void opp_eval_import(struct Opp_Stmt_Import* expr, struct Opp_Obj* obj);
void opp_eval_while(struct Opp_Stmt_While* expr, struct Opp_Obj* obj);
void opp_eval_func(struct Opp_Stmt_Func* expr, struct Opp_Obj* obj);
void opp_eval_return(struct Opp_Stmt_Ret* expr, struct Opp_Obj* obj);
void opp_eval_sub(struct Opp_Expr_Sub* expr, struct Opp_Obj* obj);
void opp_eval_array(struct Opp_Expr_Array* expr, struct Opp_Obj* obj);
void opp_eval_element(struct Opp_Expr_Element* expr, struct Opp_Obj* obj);

void opp_get_element(struct Opp_Obj* obj, char* name);
void opp_assign_element(struct Opp_Expr_Assign* expr, struct Opp_Obj* obj);

#endif