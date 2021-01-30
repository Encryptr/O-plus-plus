/** @file ast.h
 * 
 * @brief Opp AST
 *      
 * Copyright (c) 2020 Maks S
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OPP_AST
#define OPP_AST

#include <inttypes.h>
#include "../lexer/lexer.h"

union Opp_Value {
	int64_t i64val;
	double  f64val;
	char*   strval;
};

enum Opp_Node_Type {
	IGNORE,
	EBIN, ELOGIC, ECALL,
	EASSIGN, EUNARY, ESUB,
	EELEMENT, EDEREF, EADJUST,
	EADDR, ESIZEOF, EBITFIELD,
	EDOT, EBIT,

	STMT_LABEL, STMT_GOTO, STMT_IF,
	STMT_BLOCK, STMT_VAR,
	STMT_IMPORT, STMT_WHILE,
	STMT_FUNC, STMT_RET, 
	STMT_EXTERN, STMT_FOR,
	STMT_SWITCH, STMT_ENUM,
	STMT_CASE, STMT_BREAK,
	STMT_STRUCT,
};

struct Opp_Debug {
	uint32_t line;
	uint32_t colum;
};

typedef struct Opp_Debug OppDebug;

struct Opp_Type_Decl {
	struct Opp_Type_Entry* decl;
	unsigned short depth, size;
	bool unsign;
};

struct Opp_List {
	unsigned int length;
	struct Opp_Node** list;
};

struct Opp_Expr_Bin {
	enum Opp_Token tok;
	struct Opp_Node* right;
	struct Opp_Node* left;
};

struct Opp_Expr_Unary {
	enum Opp_Token type;
	union Opp_Value val;
};

struct Opp_Expr_Assign {
	enum Opp_Token op;
	struct Opp_Node* val;
	struct Opp_Node* ident;
};

struct Opp_Expr_Call {
	struct Opp_Node* callee;
	struct Opp_List* args;
};

struct Opp_Expr_Logic {
	enum Opp_Token tok;
	struct Opp_Node* right;
	struct Opp_Node* left;
};

struct Opp_Expr_Sub {
	struct Opp_Node* unary;
};

struct Opp_Expr_Element {
	struct Opp_Node* name;
	struct Opp_Node* loc;
};

struct Opp_Expr_Adjust {
	enum Opp_Token type;
	struct Opp_Node* left;
};	

struct Opp_Expr_Deref {
	struct Opp_Node* deref;
};

struct Opp_Expr_Addr {
	struct Opp_Node* addr;
};

struct Opp_Expr_Sizeof {
	struct Opp_Node* expr;
};

struct Opp_Stmt_Label {
	struct Opp_Node* name;
};

struct Opp_Stmt_Goto {
	struct Opp_Node* name;
};

struct Opp_Stmt_Switch {
	struct Opp_Node* cond;
	struct Opp_Node* block;
};

struct Opp_Stmt_Case {
	struct Opp_Node* cond;
	struct Opp_Node* stmt;	
};

struct Opp_Stmt_For {
	struct Opp_Node* decl;
	struct Opp_Node* cond;
	struct Opp_Node* expr;
	struct Opp_Node* body;
};

struct Opp_Expr_Dot {
	struct Opp_Node* left;
	struct Opp_Node* right;
};

struct Opp_Stmt_If {
	struct Opp_Node* cond;
	struct Opp_Node* then;
	struct Opp_Node* other;
};

struct Opp_Stmt_Extrn {
	struct Opp_Node* stmt;
};

struct Opp_Stmt_Block {
	struct Opp_Namespace* ns;
	unsigned int len;
	struct Opp_Node** stmts;
};

struct Opp_Stmt_Var {
	struct Opp_Type_Decl type;
	struct Opp_Node* var;
	unsigned int off;
};

struct Opp_Stmt_Import {
	struct Opp_Parser* import;
};

struct Opp_Stmt_While {
	struct Opp_Node* cond;
	struct Opp_Node* then;
};

struct Opp_Func_Args {
	struct Opp_Type_Decl type;
	struct Opp_Node* name;
};

struct Opp_Stmt_Func {
	struct Opp_Type_Decl type;
	struct Opp_Node* name;
	struct Opp_Node* body;
	unsigned int len;
	struct Opp_Node* args;
	struct Opp_Namespace* scope; // remove and move to block
};

struct Opp_Stmt_Ret {
	struct Opp_Node* value;
};

struct Opp_Stmt_Struct {
	char* name;
	unsigned int len;
	struct Opp_Node** elems; 
	struct Opp_Namespace* ns;
};

struct Opp_Node {
	enum Opp_Node_Type type;
	struct Opp_Debug debug;

	union {
		struct Opp_Expr_Bin bin_expr;
		struct Opp_Expr_Unary unary_expr;
		struct Opp_Expr_Assign assign_expr;
		struct Opp_Expr_Call call_expr;
		struct Opp_Expr_Logic logic_expr;
		struct Opp_Expr_Sub sub_expr;
		struct Opp_Expr_Adjust adjust_expr;
		struct Opp_Expr_Deref deref_expr;
		struct Opp_Expr_Addr addr_expr;
		struct Opp_Expr_Element elem_expr;
		struct Opp_Expr_Sizeof sizeof_expr;
		struct Opp_Stmt_Label label_stmt;
		struct Opp_Expr_Dot dot_expr;
		struct Opp_Stmt_If if_stmt;
		struct Opp_Stmt_Switch switch_stmt;
		struct Opp_Stmt_Case case_stmt;
		struct Opp_Stmt_For for_stmt;
		struct Opp_Stmt_Block block_stmt;
		struct Opp_Stmt_Var var_stmt;
		struct Opp_Stmt_Import import_stmt;
		struct Opp_Stmt_While while_stmt;
		struct Opp_Stmt_Func fn_stmt;
		struct Opp_Stmt_Ret ret_stmt;
		struct Opp_Stmt_Goto goto_stmt;
		struct Opp_Stmt_Extrn extrn_stmt;
		struct Opp_Stmt_Struct struct_stmt;
	};
};

#endif