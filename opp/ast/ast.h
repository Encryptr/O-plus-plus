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

enum Opp_Expr_Type {
	IGNORE,
	EBIN, ELOGIC, ECALL,
	EASSIGN, EUNARY, ESUB,
	EELEMENT, EDEREF, EADJUST,
	EADDR, ESIZEOF, EBITFIELD,
	EDOT, EBIT,
};

enum Opp_Stmt_Type {
    STMT_EXPR,
    STMT_LABEL, STMT_GOTO, STMT_IF,
	STMT_BLOCK, STMT_VAR, STMT_IMPORT, 
    STMT_WHILE, STMT_FUNC, STMT_RET, 
	STMT_EXTERN, STMT_FOR, STMT_SWITCH, 
    STMT_ENUM, STMT_CASE, STMT_BREAK,
	STMT_STRUCT,
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
struct Opp_Expr_Comma {
    struct Opp_Expr* expr;
    struct Opp_Expr* next;
};

struct Opp_Expr_Bin {
	enum Opp_Token tok;
	struct Opp_Expr* right;
	struct Opp_Expr* left;
};

struct Opp_Expr_Unary {
	enum Opp_Token type;
	union Opp_Value val;
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

struct Opp_Expr_Operator {
    enum Opp_Token tok;
	struct Opp_Expr* unary;
};

struct Opp_Expr_Element {
	struct Opp_Expr* name;
	struct Opp_Expr* loc;
};

struct Opp_Expr_Adjust {
	enum Opp_Token type;
	struct Opp_Expr* left;
};	

struct Opp_Expr_Deref {
	struct Opp_Expr* deref;
};

struct Opp_Expr_Addr {
	struct Opp_Expr* addr;
};

struct Opp_Expr_Sizeof {
	struct Opp_Expr* expr;
};

struct Opp_Expr {
    enum Opp_Expr_Type type;
    struct Opp_Debug debug;
    bool lvalue;
    union {
        struct Opp_Expr_Comma comma;
        struct Opp_Expr_Bin bin;
    } expr;
};

// Stmts
struct Opp_Stmt {
    enum Opp_Stmt_Type type;
    struct Opp_Debug debug;
    union {
        struct Opp_Expr expr_stmt;
    } stmt;
};

#endif /* OPP_AST */