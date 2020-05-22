#ifndef OPP_AST
#define OPP_AST

#include "../lexer/lexer.h"

enum Value_Type {
	VBOOL, VINT, VDOUBLE, VSTR,
	VCFUNC, VLIST, VFUNC
};

struct Opp_Value {
	enum Value_Type vtype;
	union {
		int ival;
		double dval;
		char* strval;
		_Bool bval;
	};
};

enum Opp_Expr_Type {
	EBIN, ELOGIC, ECALL,
	EASSIGN, EUNARY, ESUB
};

enum Opp_Stmt_Type {
	STMT_EXPR, STMT_IF,
	STMT_BLOCK, STMT_VAR,
	STMT_IMPORT, STMT_WHILE,
	STMT_FUNC
};

struct Opp_Expr {
	enum Opp_Expr_Type type;
	void* expr;
};

struct Opp_List {
	int size;
	struct Opp_Expr** list;
};

struct Opp_Stmt {
	enum Opp_Stmt_Type type;
	void* stmt;
};

struct Opp_Expr_Bin {
	enum Opp_Token tok;
	struct Opp_Expr* right;
	struct Opp_Expr* left;
};

struct Opp_Expr_Unary {
	enum Opp_Token type;
	struct Opp_Value val;
};

struct Opp_Expr_Assign {
	enum Opp_Token op;
	struct Opp_Expr* val;
	struct Opp_Expr* ident;
};

struct Opp_Expr_Call {
	struct Opp_Expr* callee;
	struct Opp_List* args;
};

struct Opp_Expr_Logic {
	enum Opp_Token tok;
	struct Opp_Expr* right;
	struct Opp_Expr* left;
};

struct Opp_Expr_Sub {
	struct Opp_Expr* unary;
};

struct Opp_Stmt_Expr {
	struct Opp_Expr* expr;
};

struct Opp_Stmt_If {
	struct Opp_Expr* cond;
	struct Opp_Stmt* then;
	struct Opp_Stmt* other;
};

struct Opp_Stmt_Block {
	struct Opp_Stmt** stmts;
};

struct Opp_Stmt_Var {
	struct Opp_Expr* ident;
};

struct Opp_Stmt_Import {
	struct Opp_Expr* ident;
};

struct Opp_Stmt_While {
	struct Opp_Expr* cond;
	struct Opp_Stmt* then;
};

struct Opp_Stmt_Func {
	struct Opp_Expr* name;
	struct Opp_Stmt* body;
	struct Opp_List* args;
};

#endif