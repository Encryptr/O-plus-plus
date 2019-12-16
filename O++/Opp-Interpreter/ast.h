#ifndef OPP_AST
#define OPP_AST

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

enum Ast_Type {
	AST_STMT, AST_INTEGER, AST_REAL, AST_STRING,
	AST_CONST_INT, AST_CONST_REAL, AST_CONST_STRING, 
	AST_LIST, AST_STMT_END, AST_END
};

enum Opp_Type {
	T_INVALID, T_INTEGER, T_REAL, T_STRING
};

struct Opp_Value {
	enum Opp_Type val_type;
	union {
		int ival;
		double real;
		char* strval;
	};

	struct Opp_Value* next; 
};

struct Opp_Node {
	enum Ast_Type type;

	struct Opp_Value value;

	struct Opp_Node* lt;
	struct Opp_Node* rt;
};

struct Opp_Ast {
	struct Opp_Node* tree;
};

struct Opp_Node* opp_make(enum Ast_Type type, struct Opp_Value value);

struct Opp_Node* opp_new_node(enum Ast_Type type, struct Opp_Node* op1, 
							struct Opp_Node* op2, struct Opp_Value value);

struct Opp_Value null_opp_value();

void opp_debug_tree(struct Opp_Ast* oppTree);

#endif