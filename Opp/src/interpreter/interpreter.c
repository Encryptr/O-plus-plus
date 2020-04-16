#include "interpreter.h"

#define DEBUG

void opp_eval_init(struct Opp_Parser* parser)
{
	if (global_ns == NULL || parser == NULL)
		return;

	for (int stmt=0; stmt<parser->nstmt; stmt++)
	{
		struct Opp_Obj* ret_val = opp_eval(parser->statments[stmt]);

		#ifdef DEBUG
			switch (ret_val->obj_type)
			{
				case OBJ_INT: 
					printf("INT: %d\n", ret_val->oint);
					break;

				case OBJ_BOOL:
					printf("BOOL: %d\n", ret_val->obool);
					break;

				case OBJ_NONE:
					printf("NONE\n");
					break;

			}
		#endif
	}
}

struct Opp_Obj* obj_make(enum Opp_Obj_Type obj_type)
{
	struct Opp_Obj* obj = (struct Opp_Obj*)malloc(sizeof(struct Opp_Obj));

	obj->obj_type = obj_type;

	return obj;
}

struct Opp_Obj* opp_eval(struct Opp_Stmt* stmt)
{
	switch (stmt->type)
	{
		case STMT_EXPR: {
			struct Opp_Stmt_Expr* temp = (struct Opp_Stmt_Expr*)(stmt->stmt);
			struct Opp_Expr* expr = (struct Opp_Expr*)(temp->expr);
			return opp_eval_expr(expr);
		}

		case STMT_IF: {
			struct Opp_Stmt_If* temp = (struct Opp_Stmt_If*)(stmt->stmt);
			return opp_eval_ifstmt(temp);
		}

		case STMT_BLOCK: {
			struct Opp_Stmt_Block* temp = (struct Opp_Stmt_Block*)(stmt->stmt);
			return opp_eval_block(temp);
		}

		case STMT_VAR: {
			struct Opp_Stmt_Var* temp = (struct Opp_Stmt_Var*)(stmt->stmt);
			return opp_eval_var(temp);
		}
	}
	return NULL;
}

struct Opp_Obj* opp_eval_expr(struct Opp_Expr* expr)
{
	struct Opp_Obj* literal = NULL;

	switch (expr->type)
	{
		case EBIN: {
			struct Opp_Expr_Bin* temp = (struct Opp_Expr_Bin*)(expr->expr);
			return opp_eval_bin(temp);
		}

		case ELOGIC: {
			struct Opp_Expr_Logic* temp = (struct Opp_Expr_Logic*)(expr->expr);
			return opp_eval_logic(temp);
		}

		case EUNARY: {
			struct Opp_Expr_Unary* temp = (struct Opp_Expr_Unary*)(expr->expr);

			switch (temp->type)
			{
				case INTEGER:
					literal = obj_make(OBJ_INT);
					literal->oint = temp->val.ival;
					return literal;

				case FLOAT:
					literal = obj_make(OBJ_FLOAT);
					literal->ofloat = temp->val.dval;
					return literal;

				case IDENT: {
					unsigned int loc = 0;
					if (temp->val.strval != NULL)
						loc = hash_str(temp->val.strval, current_ns->inside);
					else opp_error(NULL, "Error with ident");

					// Search Identifier scope
					struct Namespace* search = current_ns;
					while (search != NULL)
					{
						if (search->inside->list[loc] != NULL) {
							switch (search->inside->list[loc]->type)
							{
								case VINT:
									literal = obj_make(OBJ_INT);
									literal->oint = search->inside->list[loc]->value.ival;
									break;
							}
							return literal;
						}
						search = search->parent;
					}
					opp_error(NULL, "Identifier '%s' not declared", temp->val.strval);
				}

				case STR:

				break;

				case TTRUE:
					literal = obj_make(OBJ_BOOL);
					literal->obool = 1;
					return literal;

				case TFALSE:
					literal = obj_make(OBJ_BOOL);
					literal->obool = 0;
					return literal;
			}
		}

		case ECALL: {
			struct Opp_Expr_Call* temp = (struct Opp_Expr_Call*)(expr->expr);
			return opp_eval_call(temp);
		}


		case EASSIGN: {
			struct Opp_Expr_Assign* temp = (struct Opp_Expr_Assign*)(expr->expr);
			return opp_eval_assign(temp);
		}
	}

	return NULL;
}

struct Opp_Obj* opp_eval_bin(struct Opp_Expr_Bin* expr)
{
	struct Opp_Obj* obj = NULL;
	struct Opp_Obj* left = NULL;
	struct Opp_Obj* right = NULL;

	switch (expr->tok)
	{
		case TADD: {
			left = opp_eval_expr(expr->left);
			right = opp_eval_expr(expr->right);

			switch (left->obj_type)
			{
				case OBJ_INT:
					if (right->obj_type == OBJ_INT)
						left->oint += right->oint;
					else if (right->obj_type == OBJ_FLOAT)
						left->oint += (int)right->ofloat;
					break;

				case OBJ_FLOAT:
					if (right->obj_type == OBJ_INT)
						left->ofloat += right->oint;
					else if (right->obj_type == OBJ_FLOAT)
						left->ofloat += right->ofloat;
					break;

				default:
					opp_error(NULL, "Attempting to add invalid types...");
			}

			return left;
		}

		case TMIN: {
			left = opp_eval_expr(expr->left);
			right = opp_eval_expr(expr->right);

			switch (left->obj_type)
			{
				case OBJ_INT:
					if (right->obj_type == OBJ_INT)
						left->oint -= right->oint;
					else if (right->obj_type == OBJ_FLOAT)
						left->oint -= (int)right->ofloat;
					break;

				case OBJ_FLOAT:
					if (right->obj_type == OBJ_INT)
						left->ofloat -= right->oint;
					else if (right->obj_type == OBJ_FLOAT)
						left->ofloat -= right->ofloat;
					break;

				default:
					opp_error(NULL, "Attempting to add invalid types...");
			}

			return left;
		}

		case TMUL: {
			left = opp_eval_expr(expr->left);
			right = opp_eval_expr(expr->right);

			switch (left->obj_type)
			{
				case OBJ_INT:
					if (right->obj_type == OBJ_INT)
						left->oint *= right->oint;
					else if (right->obj_type == OBJ_FLOAT)
						left->oint *= (int)right->ofloat;
					break;

				case OBJ_FLOAT:
					if (right->obj_type == OBJ_INT)
						left->ofloat *= right->oint;
					else if (right->obj_type == OBJ_FLOAT)
						left->ofloat *= right->ofloat;
					break;

				default:
					opp_error(NULL, "Attempting to add invalid types...");
			}

			return left;
		}

		case TDIV: {
			left = opp_eval_expr(expr->left);
			right = opp_eval_expr(expr->right);

			switch (left->obj_type)
			{
				case OBJ_INT:
					if (right->obj_type == OBJ_INT)
						left->oint /= right->oint;
					else if (right->obj_type == OBJ_FLOAT)
						left->oint /= (int)right->ofloat;
					break;

				case OBJ_FLOAT:
					if (right->obj_type == OBJ_INT)
						left->ofloat /= right->oint;
					else if (right->obj_type == OBJ_FLOAT)
						left->ofloat /= right->ofloat;
					break;

				default:
					opp_error(NULL, "Attempting to add invalid types...");
			}

			return left;
		}

		case TMOD: {
			left = opp_eval_expr(expr->left);
			right = opp_eval_expr(expr->right);

			switch (left->obj_type)
			{
				case OBJ_INT:
					if (right->obj_type == OBJ_INT)
						left->oint %= right->oint;
					else if (right->obj_type == OBJ_FLOAT)
						left->oint %= (int)right->ofloat;
					break;

				default:
					opp_error(NULL, "Attempting to add invalid types...");
			}

			return left;
		}
	}
	return NULL;
}

struct Opp_Obj* opp_eval_logic(struct Opp_Expr_Logic* expr)
{
	struct Opp_Obj* obj = NULL;
	struct Opp_Obj* left = NULL;
	struct Opp_Obj* right = NULL;

	switch (expr->tok)
	{
		case TOR: {
			left = opp_eval_expr(expr->left);
			right = opp_eval_expr(expr->right);

			if (left->obj_type != OBJ_BOOL && right->obj_type != OBJ_BOOL)
				opp_error(NULL, "Cannot do || on non booleans");
			left->obool = (left->obool || right->obool);
			left->obj_type = OBJ_BOOL;

			return left;
		}

		case TAND: {
			left = opp_eval_expr(expr->left);
			right = opp_eval_expr(expr->right);

			if (left->obj_type != OBJ_BOOL && right->obj_type != OBJ_BOOL)
				opp_error(NULL, "Cannot preform opertaion &&");
			left->obool = (left->obool && right->obool);
			left->obj_type = OBJ_BOOL;

			return left;
		}

		case TEQEQ: {
			left = opp_eval_expr(expr->left);
			right = opp_eval_expr(expr->right);

			if (left->obj_type == OBJ_INT && right->obj_type == OBJ_INT)
				left->obool = (left->oint == right->oint);

			else if (left->obj_type == OBJ_FLOAT && right->obj_type == OBJ_FLOAT) {
				if (left->ofloat == right->ofloat)
					left->obool = 1;
				else 
					left->obool = 0; 
			}

			else if (left->obj_type == OBJ_BOOL && right->obj_type == OBJ_BOOL)
				left->obool = (left->obool == right->obool);

			else if (left->obj_type != right->obj_type) {
				printf("Mixing types in '==' expression causing false\n");
				left->obool = 0;
			}
			left->obj_type = OBJ_BOOL;

			return left;
		}

	}
	return NULL;
}

struct Opp_Obj* opp_eval_call(struct Opp_Expr_Call* expr)
{
	struct Opp_Obj* obj = NULL;
	struct Opp_Expr* in = (struct Opp_Expr*)(expr->callee);

	if (in->type != EUNARY)
		opp_error(NULL, "Expected identifier for func call");

	struct Opp_Expr_Unary* unary = (struct Opp_Expr_Unary*)(in->expr);

	if (unary->val.strval == NULL)
		opp_error(NULL, "Expected identifier for func call");
	
	int func_type = env_get_type(current_ns->inside, unary->val.strval);

	if (func_type == -1)
		opp_error(NULL, "Undefined func call '%s'", unary->val.strval);

	if (func_type == VCFUNC)
	{
		void (*func)(struct Opp_List* args) = env_get_cfn(current_ns->inside, unary->val.strval);
		func(expr->args);
	}

	obj = obj_make(OBJ_BOOL);
	obj->obool = 1;

	return obj;
}

struct Opp_Obj* opp_eval_ifstmt(struct Opp_Stmt_If* expr)
{
	struct Opp_Obj* none = obj_make(OBJ_NONE);
	struct Opp_Obj* result = opp_eval_expr(expr->cond);

	if (result->obj_type != OBJ_BOOL)
		opp_error(NULL, "Expected expression in if statement");

	if (result->obool == 1)
		opp_eval(expr->then);
	else if (result->obool == 0 && expr->other != NULL)
		opp_eval(expr->other);

	return none;
}

struct Opp_Obj* opp_eval_block(struct Opp_Stmt_Block* expr)
{
	struct Opp_Obj* none = obj_make(OBJ_NONE);
	struct Namespace* temp = current_ns;
	struct Namespace* new_ns = init_namespace("block", temp);
	current_ns = new_ns;

	int i = 0;
	while (expr->stmts[i] != NULL)
	{
		opp_eval(expr->stmts[i]);
		i++;
	}

	for (int a=0; a<new_ns->inside->size; a++)
		free(new_ns->inside->list[a]);
	free(new_ns->inside);
	free(new_ns);

	current_ns = temp;
	
	return none;
}

struct Opp_Obj* opp_eval_assign(struct Opp_Expr_Assign* expr)
{
	struct Opp_Obj* none = obj_make(OBJ_NONE);
	struct Opp_Expr* a = (struct Opp_Expr*)(expr->ident);
	struct Opp_Expr* value = (struct Opp_Expr*)(expr->val);

	if (a->type != EUNARY)
		opp_error(NULL, "Expected identifier in assigment");

	struct Opp_Expr_Unary* b = (struct Opp_Expr_Unary*)(a->expr);
	struct Opp_Obj* final_val = opp_eval_expr(value);

	if (b->type != IDENT)
		opp_error(NULL, "Expected identifier in assigment");

	unsigned int loc = hash_str(b->val.strval, current_ns->inside);
	struct Namespace* search = current_ns;

	while (search != NULL)
	{
		if (search->inside->list[loc] != NULL) break;
		search = search->parent;
	}

	int type = env_get_type(search->inside, b->val.strval);
	if (type == -1)
		opp_error(NULL, "Undefined identifier in assign '%s'", b->val.strval);


	switch (expr->op)
	{
		case TEQ: {
			if (type == VINT && final_val->obj_type == OBJ_INT)
				env_new_int(search->inside, b->val.strval, final_val->oint);
			else if (type == VDOUBLE && final_val->obj_type == OBJ_FLOAT)
				env_new_dbl(search->inside, b->val.strval, final_val->ofloat);
			else 
				opp_error(NULL, "Error assigning value to var '%s'", b->val.strval);
		}
	}

	return none;
}

struct Opp_Obj* opp_eval_var(struct Opp_Stmt_Var* expr)
{
	struct Opp_Obj* none = obj_make(OBJ_NONE);
	struct Opp_Expr* a = (struct Opp_Expr*)(expr->ident);

	if (a->type != EASSIGN)
		opp_error(NULL, "Expected expression to assign a variable var <ident> '=' <expr>");

	struct Opp_Expr_Assign* b = (struct Opp_Expr_Assign*)(a->expr);
	struct Opp_Expr* c = (struct Opp_Expr*)(b->val);
	struct Opp_Expr* d = (struct Opp_Expr*)(b->ident);
	struct Opp_Expr_Unary* e = (struct Opp_Expr_Unary*)(d->expr);
	struct Opp_Obj* new_val = opp_eval_expr(c);

	switch (new_val->obj_type)
	{
		case OBJ_INT:
			env_new_int(current_ns->inside, e->val.strval, new_val->oint);
			break;

	}
	
	return none;
}
