/** @file ast.h
 * 
 * @brief Opp AST
 *      
 * Copyright (c) 2021 Maks S
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

enum Opp_Expr_Type {
	EXPR_BIN, EXPR_LOGIC, EXPR_CALL,
	EXPR_ASSIGN, EXPR_UNARY, EXPR_PREFIX,
	EXPR_TERNARY, EXPR_BIT, EXPR_CAST,
};

enum Opp_Stmt_Type {
    STMT_EXPR, STMT_LABEL, STMT_GOTO, 
    STMT_IF, STMT_BLOCK, STMT_DECL, 
	STMT_WHILE, STMT_FUNC, STMT_RET, 
	STMT_EXTERN, STMT_FOR, STMT_DOWHILE,
	STMT_SWITCH, STMT_ENUM, STMT_CASE, 
	STMT_BREAK, STMT_STRUCT, STMT_TYPEDEF, 
	STMT_NOP,
};

struct Opp_Debug {
	uint32_t line;
	uint32_t colum;
};

struct Opp_Type_Decl {
	struct Opp_Type_Entry* decl;
	unsigned short depth, size;
	bool unsign;
};

struct Opp_Expr;
struct Opp_Stmt;

// Exprs
struct Opp_Expr_Bin {
	enum Opp_Token tok;
	struct Opp_Expr* right;
	struct Opp_Expr* left;
};

struct Opp_Expr_Ternary {
	struct Opp_Expr* cond;
	struct Opp_Expr* then;
	struct Opp_Expr* other;
};

struct Opp_Expr_Unary {
	enum Opp_Token type;
	union Opp_Value val;
};

struct Opp_Expr_Call {
	struct Opp_Expr* callee;
	struct Opp_List* args;
};

struct Opp_Expr_Operator {
    enum Opp_Token tok;
	struct Opp_Expr* unary;
};

struct Opp_Expr_Element {
	struct Opp_Expr* name;
	struct Opp_Expr* loc;
};

struct Opp_Expr_Prefix {
	enum Opp_Token tok;
	struct Opp_Expr* expr;
};

struct Opp_Expr_Cast {
	struct Opp_Type* type;
	struct Opp_Expr* expr;
};

struct Opp_Expr {
    enum Opp_Expr_Type type;
    struct Opp_Debug debug;
    bool lvalue;
    union {
        struct Opp_Expr_Bin bin;
        struct Opp_Expr_Unary unary;
        struct Opp_Expr_Call call;
		struct Opp_Expr_Prefix prefix;
		struct Opp_Expr_Cast cast;
    } expr;
	struct Opp_Expr* next;
};

// Stmts
struct Opp_Stmt_Decl {
	char* name;
	struct Opp_Type* type;
	// initializer
	struct Opp_Stmt* next;
};

struct Opp_Stmt_Func {
	char* name;
	struct Opp_Type* type;
	struct Opp_Stmt* body;
};

struct Opp_Stmt_Block {
	struct Opp_Stmt** stmt;
	unsigned int len;
};

struct Opp_Stmt_If {
	struct Opp_Expr* cond;
	struct Opp_Stmt* then;
	struct Opp_Stmt* other;
};

struct Opp_Stmt_While {
	struct Opp_Expr* cond;
	struct Opp_Stmt* then;
};

struct Opp_Stmt_For {
	struct Opp_Expr* init;
	struct Opp_Expr* cond;
	struct Opp_Expr* expr;
};

struct Opp_Stmt_Atom {
	enum Opp_Token tok;
};

struct Opp_Stmt {
    enum Opp_Stmt_Type type;
    struct Opp_Debug debug;
    union {
        struct Opp_Expr* expr_stmt;
        struct Opp_Stmt_Decl decl;
        struct Opp_Stmt_Func func;
        struct Opp_Stmt_Block block;
        struct Opp_Stmt_If if_stmt;
        struct Opp_Stmt_While while_stmt;
        struct Opp_Stmt_For for_stmt;
        struct Opp_Stmt_Atom atom;
    } stmt;
};

#endif /* OPP_AST */