#ifndef OPP_AST
#define OPP_AST

#include "../lexer/lexer.h"

struct Opp_Node;

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
		struct Opp_List* array;
	};
};

struct Opp_List {
	unsigned int size;
	struct Opp_Value* list;
};

enum Opp_Expr_Type {
	EBIN, EUNARY
};

enum Opp_Stmt_Type {
	STMT_EXPR, 
};

struct Opp_Expr {
	enum Opp_Expr_Type type;
	void* expr;
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

struct Opp_Stmt_Expr {
	struct Opp_Expr* expr;
};

#endif