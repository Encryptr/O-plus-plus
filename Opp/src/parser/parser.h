#ifndef OPP_PARSER
#define OPP_PARSER

#include <stdio.h>
#include <stdlib.h>

#include "../lexer/lexer.h"
#include "../opp.h"
#include "../util/error.h"
#include "ast.h"

struct Opp_Parser {
	struct Opp_Stmt** statments;
	enum Opp_Mode mode;
	int nstmt;
};

struct Opp_Parser* opp_parse_init(struct Opp_Scan* s);
struct Opp_Expr* opp_new_expr(enum Opp_Expr_Type type, void* expr);
struct Opp_Stmt* opp_new_stmt(enum Opp_Stmt_Type type, void* stmt);

struct Opp_Stmt* opp_parse_statment(struct Opp_Scan* s);
struct Opp_Stmt* opp_parse_expression(struct Opp_Scan* s);

// Expressions
struct Opp_Expr* opp_parse_allign(struct Opp_Scan* s);
struct Opp_Expr* opp_parse_or(struct Opp_Scan* s);
struct Opp_Expr* opp_parse_and(struct Opp_Scan* s);
struct Opp_Expr* opp_parse_relation(struct Opp_Scan* s);
struct Opp_Expr* opp_parse_relation2(struct Opp_Scan* s);
struct Opp_Expr* opp_parse_expr(struct Opp_Scan* s);
struct Opp_Expr* opp_parse_expr2(struct Opp_Scan* s);
struct Opp_Expr* opp_parse_before(struct Opp_Scan* s);
struct Opp_Expr* opp_parse_prefix(struct Opp_Scan* s);
struct Opp_List* opp_parse_args(struct Opp_Scan* s);
struct Opp_Expr* opp_parse_unary(struct Opp_Scan* s);

// Statements
struct Opp_Stmt* opp_parse_ifstmt(struct Opp_Scan* s);
struct Opp_Stmt* opp_parse_block(struct Opp_Scan* s);
struct Opp_Stmt* opp_parse_var(struct Opp_Scan* s);
struct Opp_Stmt* opp_parse_import(struct Opp_Scan* s);
struct Opp_Stmt* opp_parse_while(struct Opp_Scan* s);
struct Opp_Stmt* opp_parse_func(struct Opp_Scan* s);
struct Opp_Stmt* opp_parse_return(struct Opp_Scan* s);

#endif