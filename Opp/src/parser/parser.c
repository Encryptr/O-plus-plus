#include "parser.h"

#define DEBUG

struct Opp_Parser* opp_parse_init(struct Opp_Scan* s)
{
	struct Opp_Parser* parser = (struct Opp_Parser*)malloc(sizeof(struct Opp_Parser));

	parser->nstmt = 0;
	parser->statments = calloc(10, sizeof(*parser->statments));

	while (s->tok != FEND)
	{
		opp_next(s);
		if (s->tok == FEND) 
			break;

		struct Opp_Stmt* stmt = opp_parse_statment(s);

		parser->statments[parser->nstmt] = stmt;
		parser->nstmt++;

		#ifdef DEBUG
			// struct Opp_Stmt_Expr* a = (struct Opp_Stmt_Expr*)(parser->statments[0]->stmt);
			// struct Opp_Expr* b = (struct Opp_Expr*)(a->expr);
			// struct Opp_Expr_Bin* c = (struct Opp_Expr_Bin*)(b->expr);

			// printf("==>%d\n", c->tok);
		#endif
	}

	return parser;
}

struct Opp_Expr* opp_new_expr(enum Opp_Expr_Type type, void* expr)
{
	struct Opp_Expr* new_expr = (struct Opp_Expr*)malloc(sizeof(struct Opp_Expr));

	if (new_expr == NULL)
		internal_error("Malloc fail", 1);

	new_expr->type = type;
	new_expr->expr = expr;

	return new_expr;
}

struct Opp_Stmt* opp_new_stmt(enum Opp_Stmt_Type type, void* stmt)
{
	struct Opp_Stmt* new_stmt = (struct Opp_Stmt*)malloc(sizeof(struct Opp_Stmt));

	if (new_stmt == NULL)
		internal_error("Malloc fail", 1);

	new_stmt->type = type;
	new_stmt->stmt = stmt;

	return new_stmt;
}

struct Opp_Stmt* opp_parse_statment(struct Opp_Scan* s)
{
	switch (s->tok)
	{
		default:
			return opp_parse_expression(s);
	}
}

struct Opp_Stmt* opp_parse_expression(struct Opp_Scan* s)
{
	struct Opp_Expr* new_expr = opp_parse_allign(s);
	struct Opp_Stmt_Expr* new_stmt = (struct Opp_Stmt_Expr*)malloc(sizeof(struct Opp_Stmt_Expr));
	new_stmt->expr = new_expr;
	if (new_stmt == NULL)
		internal_error("Malloc Fail", 1);

	struct Opp_Stmt* stmt = opp_new_stmt(STMT_EXPR, new_stmt);

	if (s->tok != TSEMICOLON) 
		opp_error(s, "Expected ';' after expression");

	return stmt;
}

struct Opp_Expr* opp_parse_allign(struct Opp_Scan* s)
{
	struct Opp_Expr* left = opp_parse_or(s);
	struct Opp_Expr* right = NULL;
	int operator = s->tok;

	while (operator == TADDEQ || operator == TMINEQ || operator == TEQ
			|| operator == TDIVEQ || operator == TMULEQ || operator == TMODEQ)
	{
		opp_next(s);
		right = opp_parse_or(s);

		struct Opp_Expr_Bin* temp = (struct Opp_Expr_Bin*)malloc(sizeof(struct Opp_Expr_Bin));
		if (!temp)
			internal_error("Malloc Fail", 1);
		temp->right = right;
		temp->left = left;
		temp->tok = operator;

		left = opp_new_expr(EBIN, &temp);

		operator = s->tok;
	}
	return left;
}

struct Opp_Expr* opp_parse_or(struct Opp_Scan* s)
{
	struct Opp_Expr* left = opp_parse_and(s);
	struct Opp_Expr* right = NULL;
	int operator = s->tok;

	while (operator == TOR)
	{
		opp_next(s);
		right = opp_parse_and(s);

		struct Opp_Expr_Bin* temp = (struct Opp_Expr_Bin*)malloc(sizeof(struct Opp_Expr_Bin));
		if (!temp)
			internal_error("Malloc Fail", 1);
		temp->right = right;
		temp->left = left;
		temp->tok = TOR;

		left = opp_new_expr(EBIN, temp);

		operator = s->tok;
	}
	return left;
}

struct Opp_Expr* opp_parse_and(struct Opp_Scan* s)
{
	struct Opp_Expr* left = opp_parse_relation(s);
	struct Opp_Expr* right = NULL;
	int operator = s->tok;

	while (operator == TAND)
	{
		opp_next(s);
		right = opp_parse_relation(s);

		struct Opp_Expr_Bin* temp = (struct Opp_Expr_Bin*)malloc(sizeof(struct Opp_Expr_Bin));
		if (!temp)
			internal_error("Malloc Fail", 1);
		temp->right = right;
		temp->left = left;
		temp->tok = TAND;

		left = opp_new_expr(EBIN, temp);

		operator = s->tok;
	}
	return left;
}

struct Opp_Expr* opp_parse_relation(struct Opp_Scan* s)
{
	struct Opp_Expr* left = opp_parse_relation2(s);
	struct Opp_Expr* right = NULL;
	int operator = s->tok;

	while (operator == TEQEQ || operator == TNOTEQ)
	{
		opp_next(s);
		right = opp_parse_relation2(s);

		struct Opp_Expr_Bin* temp = (struct Opp_Expr_Bin*)malloc(sizeof(struct Opp_Expr_Bin));
		if (!temp)
			internal_error("Malloc Fail", 1);
		temp->right = right;
		temp->left = left;
		temp->tok = operator;
		left = opp_new_expr(EBIN, temp);

		operator = s->tok;
	}
	return left;
}

struct Opp_Expr* opp_parse_relation2(struct Opp_Scan* s)
{
	struct Opp_Expr* left = opp_parse_expr(s);
	struct Opp_Expr* right = NULL;
	int operator = s->tok;

	while (operator == TGT || operator == TLT ||
			operator == TGE || operator == TLE)
	{
		opp_next(s);
		right = opp_parse_expr(s);

		struct Opp_Expr_Bin* temp = (struct Opp_Expr_Bin*)malloc(sizeof(struct Opp_Expr_Bin));
		if (!temp)
			internal_error("Malloc Fail", 1);
		temp->right = right;
		temp->left = left;
		temp->tok = operator;
		left = opp_new_expr(EBIN, temp);

		operator = s->tok;
	}
	return left;
}

struct Opp_Expr* opp_parse_expr(struct Opp_Scan* s)
{
	struct Opp_Expr* left = opp_parse_expr2(s);
	struct Opp_Expr* right = NULL;
	int operator = s->tok;

	while (operator == TADD || operator == TMIN)
	{
		opp_next(s);
		right = opp_parse_expr2(s);

		struct Opp_Expr_Bin* temp = (struct Opp_Expr_Bin*)malloc(sizeof(struct Opp_Expr_Bin));
		if (!temp)
			internal_error("Malloc Fail", 1);
		temp->right = right;
		temp->left = left;
		temp->tok = operator;
		left = opp_new_expr(EBIN, temp);

		operator = s->tok;
	}
	return left;
}

struct Opp_Expr* opp_parse_expr2(struct Opp_Scan* s)
{
	struct Opp_Expr* left = opp_parse_unary(s);
	struct Opp_Expr* right = NULL;

	opp_next(s);
	int operator = s->tok;
	while (operator == TDIV || operator == TMUL || operator == TMOD)
	{
		opp_next(s);
		right = opp_parse_unary(s);

		struct Opp_Expr_Bin* temp = (struct Opp_Expr_Bin*)malloc(sizeof(struct Opp_Expr_Bin));
		if (!temp)
			internal_error("Malloc Fail", 1);
		temp->right = right;
		temp->left = left;
		temp->tok = operator;
		left = opp_new_expr(EBIN, temp);

		opp_next(s);
		operator = s->tok;
	}
	return left;
}

struct Opp_Expr* opp_parse_unary(struct Opp_Scan* s)
{
	struct Opp_Expr_Unary* unary = (struct Opp_Expr_Unary*)malloc(sizeof(struct Opp_Expr_Unary));
	struct Opp_Expr* expr = NULL;

	switch (s->tok)
	{
		case IDENT:
			unary->type = IDENT;
			unary->val.strval = (char*)malloc(sizeof(char)*strlen(s->lexeme));
			strcpy(s->lexeme, unary->val.strval);
			break;

		case INTEGER:
			unary->type = INTEGER;
			unary->val.ival = atoi(s->lexeme);
			break;

		case FLOAT:
			unary->type = FLOAT;
			unary->val.dval = atof(s->lexeme);
			break;

		case STR:
			unary->type = STR;
			unary->val.strval = (char*)malloc(sizeof(char)*strlen(s->lexeme));
			strcpy(s->lexeme, unary->val.strval);
			break;

		case TTRUE: 
			unary->type = TTRUE;
			unary->val.bval = 1;
			break;

		case TFALSE:
			unary->type = TFALSE;
			unary->val.bval = 0;
			break;

		case TOPENP: {
			opp_next(s);
			struct Opp_Expr* paran = opp_parse_allign(s);
			if (s->tok != TCLOSEP)
				opp_error(s, "Expeceted ')'");
			return paran;
		}

		default:
			opp_error(s, "Invalid type used in expression");
	}
	expr = opp_new_expr(EUNARY, unary);

	return expr;
}