#include "parser.h"

#define DEBUG

struct Opp_Parser* opp_parse_init(struct Opp_Scan* s)
{
	struct Opp_Parser* parser = (struct Opp_Parser*)malloc(sizeof(struct Opp_Parser));

	parser->nstmt = 0;
	parser->statments = calloc(100, sizeof(*parser->statments));

	while (s->tok != FEND)
	{
		opp_next(s);
		if (s->tok == FEND) 
			break;

		struct Opp_Stmt* stmt = opp_parse_statment(s);

		parser->statments[parser->nstmt] = stmt;
		parser->nstmt++;
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
		case TFUNC:
			return opp_parse_func(s);

		case TIF:
			return opp_parse_ifstmt(s);

		case TOPENC:
			return opp_parse_block(s);

		case TVAR:
			return opp_parse_var(s);

		case TIMPORT:
			return opp_parse_import(s);

		case TWHILE:
			return opp_parse_while(s);

		case TRET:
			return opp_parse_return(s);

		default:
			return opp_parse_expression(s);
	}
}

struct Opp_Stmt* opp_parse_while(struct Opp_Scan* s)
{
	struct Opp_Stmt_While* loop = NULL;
	struct Opp_Expr* cond = NULL;
	struct Opp_Stmt* stmt = NULL;

	opp_next(s);
	cond = opp_parse_allign(s);
	stmt = opp_parse_statment(s);

	loop = (struct Opp_Stmt_While*)malloc(sizeof(struct Opp_Stmt_While));
	loop->cond = cond;
	loop->then = stmt;

	return opp_new_stmt(STMT_WHILE, loop);
}

struct Opp_Stmt* opp_parse_return(struct Opp_Scan* s)
{
	struct Opp_Stmt_Ret* ret = NULL;
	struct Opp_Expr* val = NULL;

	opp_next(s);
	val = opp_parse_allign(s);

	ret = (struct Opp_Stmt_Ret*)malloc(sizeof(struct Opp_Stmt_Ret));
	ret->value = val;

	if (s->tok != TSEMICOLON)
		opp_error(s, "Expected ';' after return statement");

	return opp_new_stmt(STMT_RET, ret);
}

struct Opp_Stmt* opp_parse_func(struct Opp_Scan* s)
{
	struct Opp_Stmt_Func* func = NULL;
	struct Opp_Expr* name = NULL;
	struct Opp_Stmt* body = NULL;
	struct Opp_List* args = NULL;

	opp_next(s);

	if (s->tok != IDENT)
		opp_error(s, "Expected name for function");

	name = opp_parse_unary(s);
	opp_next(s);

	args = opp_parse_args(s);
	opp_next(s);
	body = opp_parse_statment(s);

	func = (struct Opp_Stmt_Func*)malloc(sizeof(struct Opp_Stmt_Func));
	func->name = name;
	func->args = args;
	func->body = body;

	return opp_new_stmt(STMT_FUNC, func);
}

struct Opp_Stmt* opp_parse_import(struct Opp_Scan* s)
{
	struct Opp_Stmt_Import* import = NULL;
	struct Opp_Expr* ident = NULL;

	opp_next(s);
	ident = opp_parse_allign(s);

	import = (struct Opp_Stmt_Import*)malloc(sizeof(struct Opp_Stmt_Import));
	import->ident = ident;

	if (s->tok != TSEMICOLON)
		opp_error(s, "Expected ';' after import");

	return opp_new_stmt(STMT_IMPORT, import);
}

struct Opp_Stmt* opp_parse_var(struct Opp_Scan* s)
{
	struct Opp_Stmt_Var* var = NULL;
	struct Opp_Expr* ident = NULL;

	opp_next(s);
	ident = opp_parse_allign(s);

	var = (struct Opp_Stmt_Var*)malloc(sizeof(struct Opp_Stmt_Var));
	var->ident = ident;

	if (s->tok != TSEMICOLON)
		opp_error(s, "Expected ';' after var decleration");

	return opp_new_stmt(STMT_VAR, var);
}

struct Opp_Stmt* opp_parse_ifstmt(struct Opp_Scan* s)
{
	struct Opp_Expr* cond = NULL;
	struct Opp_Stmt* then = NULL;
	struct Opp_Stmt_If* ifstmt = NULL;
	struct Opp_Stmt* elsestmt = NULL;

	opp_next(s);
	cond = opp_parse_allign(s);
	then = opp_parse_statment(s);

	// opp_next(s);

	if (s->tok == TELSE) {
		opp_next(s);
		elsestmt = opp_parse_statment(s);
	}

	ifstmt = (struct Opp_Stmt_If*)malloc(sizeof(struct Opp_Stmt_If));
	ifstmt->cond = cond;
	ifstmt->then = then;
	ifstmt->other = elsestmt;

	return opp_new_stmt(STMT_IF, ifstmt);
}

struct Opp_Stmt* opp_parse_block(struct Opp_Scan* s)
{
	struct Opp_Stmt_Block* block = (struct Opp_Stmt_Block*)malloc(sizeof(struct Opp_Stmt_Block));
	block->stmts = (struct Opp_Stmt**)malloc(sizeof(struct Opp_Stmt*)*100);
	if (block == NULL)
		internal_error("Malloc Fail", 1);

	opp_next(s);
	int i = 0;
	int max = 100;
	while (s->tok != TCLOSEC && s->tok != FEND) {
		if (i == max) {
			struct Opp_Stmt** temp = NULL;
			temp = realloc(block, 100 * sizeof(struct Opp_Stmt*));
			if (temp == NULL)
				internal_error("Malloc Fail", 1);
			else block->stmts = temp;
			max = max + 100;
		}
		block->stmts[i] = opp_parse_statment(s);
		i++;
		// if (s->tok == TCLOSEC) break;
		opp_next(s);
	}

	if (s->tok != TCLOSEC)
		opp_error(s, "Expected terminating '}' in stmt");

	return opp_new_stmt(STMT_BLOCK, block);
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

		struct Opp_Expr_Assign* temp = (struct Opp_Expr_Assign*)malloc(sizeof(struct Opp_Expr_Assign));
		if (!temp)
			internal_error("Malloc Fail", 1);
		temp->ident = left;
		temp->val = right;
		temp->op = operator;

		left = opp_new_expr(EASSIGN, temp);

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

		struct Opp_Expr_Logic* temp = (struct Opp_Expr_Logic*)malloc(sizeof(struct Opp_Expr_Logic));
		if (!temp)
			internal_error("Malloc Fail", 1);
		temp->right = right;
		temp->left = left;
		temp->tok = TOR;

		left = opp_new_expr(ELOGIC, temp);

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

		struct Opp_Expr_Logic* temp = (struct Opp_Expr_Logic*)malloc(sizeof(struct Opp_Expr_Logic));
		if (!temp)
			internal_error("Malloc Fail", 1);
		temp->right = right;
		temp->left = left;
		temp->tok = TAND;

		left = opp_new_expr(ELOGIC, temp);

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

		struct Opp_Expr_Logic* temp = (struct Opp_Expr_Logic*)malloc(sizeof(struct Opp_Expr_Logic));
		if (!temp)
			internal_error("Malloc Fail", 1);
		temp->right = right;
		temp->left = left;
		temp->tok = operator;
		left = opp_new_expr(ELOGIC, temp);

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

		struct Opp_Expr_Logic* temp = (struct Opp_Expr_Logic*)malloc(sizeof(struct Opp_Expr_Logic));
		if (!temp)
			internal_error("Malloc Fail", 1);
		temp->right = right;
		temp->left = left;
		temp->tok = operator;
		left = opp_new_expr(ELOGIC, temp);

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
	// struct Opp_Expr* left = opp_parse_prefix(s);
	struct Opp_Expr* left = opp_parse_before(s);
	struct Opp_Expr* right = NULL;

	int operator = s->tok;
	while (operator == TDIV || operator == TMUL || operator == TMOD)
	{
		opp_next(s);
		// right = opp_parse_prefix(s);
		right = opp_parse_before(s);

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

struct Opp_Expr* opp_parse_before(struct Opp_Scan* s)
{
	struct Opp_Expr* value = NULL;
	int operator = s->tok;
	
	if (operator == TMIN)
	{
		struct Opp_Expr_Sub* temp = (struct Opp_Expr_Sub*)malloc(sizeof(struct Opp_Expr_Sub));
		opp_next(s);
		temp->unary = opp_parse_expr2(s);
		value = opp_new_expr(ESUB, temp);

		return value;
	}

	return opp_parse_prefix(s);
}

struct Opp_Expr* opp_parse_prefix(struct Opp_Scan* s)
{
	struct Opp_Expr* left = opp_parse_unary(s);

	opp_next(s);
	int operator = s->tok;
	while (operator == TOPENP)
	{
		// opp_next(s);
		struct Opp_Expr_Call* call = (struct Opp_Expr_Call*)malloc(sizeof(struct Opp_Expr_Call));
		call->callee = left;
		call->args = opp_parse_args(s);
		left = opp_new_expr(ECALL, call);

		opp_next(s);
		operator = s->tok;
	}

	return left;
}

struct Opp_List* opp_parse_args(struct Opp_Scan* s)
{
	struct Opp_List* args = (struct Opp_List*)malloc(sizeof(struct Opp_List));
	args->list = (struct Opp_Expr**)malloc(sizeof(struct Opp_Expr*)*10);

	int i = 0;

	do {
		opp_next(s);
		if (i==0 && s->tok == TCLOSEP) break;

		args->list[i] = opp_parse_allign(s);

		if (i == 10)
			opp_error(s, "No more than 10 args to func");
		i++;
		if (s->tok == TCLOSEP) break;
	} while (s->tok == TCOMMA);

	end:
	args->size = i;
	if (s->tok != TCLOSEP)
		opp_error(s, "Missing terminating ')' in func call");

	return args;
}

struct Opp_Expr* opp_parse_unary(struct Opp_Scan* s)
{
	struct Opp_Expr_Unary* unary = (struct Opp_Expr_Unary*)malloc(sizeof(struct Opp_Expr_Unary));
	struct Opp_Expr* expr = NULL;

	switch (s->tok)
	{
		case IDENT:
			unary->type = IDENT;
			unary->val.strval = (char*)malloc(sizeof(char)*strlen(s->lexeme)+1);
			strcpy(unary->val.strval, s->lexeme);
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
			strcpy(unary->val.strval, s->lexeme);
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
			opp_error(s, "Invalid type used in expression / missing type");
	}
	expr = opp_new_expr(EUNARY, unary);

	return expr;
}