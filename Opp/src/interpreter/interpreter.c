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
			printf("===>%d\n", ret_val->oint);
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

				case STR:
				case TTRUE:
				case TFALSE:
				break;
			}
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

				case OBJ_FLOAT:
					// error
					break;
			}

			return left;
		}
	}
	return NULL;
}
