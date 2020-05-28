#include "interpreter.h"

// #define ARRAY_TEST

void opp_eval_init(struct Opp_Parser* parser)
{
	if (global_ns == NULL || parser == NULL)
		return;

	for (int stmt=0; stmt<parser->nstmt; stmt++)
	{
		struct Opp_Obj ret_val = {0};
		opp_eval(parser->statments[stmt], &ret_val);

		if (parser->mode == IREPL)
			opp_repl_ret(&ret_val);
	}
}

void opp_repl_ret(struct Opp_Obj* val)
{
	if (val == NULL)
		opp_error(NULL, "Error repl return");

	switch (val->obj_type)
	{
		case OBJ_INT: 
			printf("INT: %d\n", val->oint);
			break;

		case OBJ_BOOL:
			printf("BOOL: %d\n", val->obool);
			break;

		case OBJ_STR:
			printf("STR: %s\n", val->ostr);
			break;

		case OBJ_FLOAT:
			printf("REAL: %lf\n", val->ofloat);
			break;

		// case OBJ_ARRAY:
		// 	printf("[");
		// 	for (int i = 0; i < val->arr.size; i++)
		// 	{
		// 		if (i == val->arr.size-1)
		// 			printf("%d", val->arr.iarr[i]);
		// 		else
		// 			printf("%d ", val->arr.iarr[i]);
		// 	}
			// printf("]\n");
			break;
	}
}

struct Opp_Obj* obj_make(enum Opp_Obj_Type obj_type)
{
	struct Opp_Obj* obj = (struct Opp_Obj*)malloc(sizeof(struct Opp_Obj));

	obj->obj_type = obj_type;

	return obj;
}

void opp_eval(struct Opp_Stmt* stmt, struct Opp_Obj* obj)
{
	switch (stmt->type)
	{
		case STMT_EXPR: {
			struct Opp_Stmt_Expr* temp = (struct Opp_Stmt_Expr*)(stmt->stmt);
			struct Opp_Expr* expr = (struct Opp_Expr*)(temp->expr);
			opp_eval_expr(expr, obj);
			break;
		}

		case STMT_IF: {
			struct Opp_Stmt_If* temp = (struct Opp_Stmt_If*)(stmt->stmt);
			opp_eval_ifstmt(temp, obj);
			break;
		}

		case STMT_BLOCK: {
			struct Opp_Stmt_Block* temp = (struct Opp_Stmt_Block*)(stmt->stmt);
			opp_eval_block(temp, obj);
			break;
		}

		case STMT_VAR: {
			struct Opp_Stmt_Var* temp = (struct Opp_Stmt_Var*)(stmt->stmt);
			opp_eval_var(temp, obj);
			break;
		}

		case STMT_IMPORT: {
			struct Opp_Stmt_Import* temp = (struct Opp_Stmt_Import*)(stmt->stmt);
			opp_eval_import(temp, obj);
			break;
		}

		case STMT_WHILE: {
			struct Opp_Stmt_While* temp = (struct Opp_Stmt_While*)(stmt->stmt);
			opp_eval_while(temp, obj);
			break;
		}

		case STMT_FUNC: {
			struct Opp_Stmt_Func* temp = (struct Opp_Stmt_Func*)(stmt->stmt);
			opp_eval_func(temp, obj);
			break;
		}

		case STMT_RET: {
			struct Opp_Stmt_Ret* temp = (struct Opp_Stmt_Ret*)(stmt->stmt);
			opp_eval_return(temp, obj);
			break;
		}
	}
}

void opp_eval_expr(struct Opp_Expr* expr, struct Opp_Obj* literal)
{
	switch (expr->type)
	{
		case EBIN: {
			struct Opp_Expr_Bin* temp = (struct Opp_Expr_Bin*)(expr->expr);
			opp_eval_bin(temp, literal);
			break;
		}

		case ELOGIC: {
			struct Opp_Expr_Logic* temp = (struct Opp_Expr_Logic*)(expr->expr);
			opp_eval_logic(temp, literal);
			break;
		}

		case EUNARY: {
			struct Opp_Expr_Unary* temp = (struct Opp_Expr_Unary*)(expr->expr);

			switch (temp->type)
			{
				case INTEGER:
					literal->obj_type = OBJ_INT;
					literal->oint = temp->val.ival;
					break;

				case FLOAT:
					literal->obj_type = OBJ_FLOAT;
					literal->ofloat = temp->val.dval;
					break;

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
									literal->obj_type = OBJ_INT;
									literal->oint = search->inside->list[loc]->value.ival;
									break;

								case VDOUBLE:
									literal->obj_type = OBJ_FLOAT;
									literal->ofloat = search->inside->list[loc]->value.dval;
									break;

								case VBOOL:
									literal->obj_type = OBJ_BOOL;
									literal->obool = search->inside->list[loc]->value.bval;
									break;

								case VSTR:
									literal->obj_type = OBJ_STR;
									strcpy(literal->ostr, search->inside->list[loc]->value.strval);
									break;

								// case VLIST:
								// 	literal->obj_type = OBJ_ARRAY;
								// 	literal->arr.array_type = OBJ_INT;
								// 	int i = 0;
								// 	for (struct Opp_Value* val = search->inside->list[loc]->value.next; val != NULL; val = val->next)
								// 	{
								// 		literal->arr.iarr[i] = val->ival;
								// 		i++;
								// 	}
								// 	literal->arr.size = i;
								// 	break;

								case VNONE:
									opp_error(NULL, "Attempting to evaluate a var '%s' of type none", 
										search->inside->list[loc]->key);
							}
							return;
						}
						search = search->parent;
					}
					opp_error(NULL, "Identifier '%s' not declared", temp->val.strval);
				}

				case STR:
					literal->obj_type = OBJ_STR;
					strcpy(literal->ostr, temp->val.strval);
					break;

				case TTRUE:
					literal->obj_type = OBJ_BOOL;
					literal->obool = 1;
					break;

				case TFALSE:
					literal->obj_type = OBJ_BOOL;
					literal->obool = 0;
					break;
			}
			break;
		}

		case ESUB: {
			struct Opp_Expr_Sub* temp = (struct Opp_Expr_Sub*)(expr->expr);
			opp_eval_sub(temp, literal);
			break;
		}

		case ECALL: {
			struct Opp_Expr_Call* temp = (struct Opp_Expr_Call*)(expr->expr);
			opp_eval_call(temp, literal);
			break;
		}

		case EASSIGN: {
			struct Opp_Expr_Assign* temp = (struct Opp_Expr_Assign*)(expr->expr);
			opp_eval_assign(temp, literal);
			break;
		}

		// case EARRAY: {
		// 	struct Opp_Expr_Array* temp = (struct Opp_Expr_Array*)(expr->expr);
		// 	opp_eval_array(temp, literal);
		// 	break;
		// }

		// case EELEMENT: {
		// 	struct Opp_Expr_Element* temp = (struct Opp_Expr_Element*)(expr->expr);
		// 	opp_eval_element(temp, literal);
		// 	break;
		// }
	}
}

void opp_eval_sub(struct Opp_Expr_Sub* expr, struct Opp_Obj* obj)
{
	struct Opp_Obj val = {0};
	opp_eval_expr(expr->unary, &val);

	obj->obj_type = val.obj_type;
	switch (val.obj_type) 
	{
		case OBJ_INT: 
			obj->oint = val.oint * -1; 
			break;

		case OBJ_FLOAT: 
			obj->ofloat = val.ofloat * -1.0; 
			break;

		default:
			opp_error(NULL, "Error attempting to negate invalid type");
	}
}

// void opp_eval_element(struct Opp_Expr_Element* expr, struct Opp_Obj* obj)
// {
// 	struct Opp_Expr* a = (expr->loc);
// 	struct Opp_Expr_Array* b = (a->expr);

// 	struct Opp_Expr* w = (expr->name);
// 	struct Opp_Expr_Unary* u = (w->expr);

// 	struct Opp_Obj loc = {0};

// 	opp_eval_expr(b->elements[0], &loc);

// 	if (loc.obj_type != OBJ_INT)
// 		opp_error(NULL, "Error only integers for element index '%s'", u->val.strval);

// 	if (loc.oint < 0)
// 		opp_error(NULL, "Error element value not allowed to less than 0 '%s'", u->val.strval);

// 	if (!env_get_element(current_ns->inside, u->val.strval, loc.oint, obj))
// 		opp_error(NULL, "Error getting element from array '%s'", u->val.strval);
// }

void opp_eval_bin(struct Opp_Expr_Bin* expr, struct Opp_Obj* obj)
{
	struct Opp_Obj left = {0};
	struct Opp_Obj right = {0};

	switch (expr->tok)
	{
		case TADD: {
			opp_eval_expr(expr->left, &left);
			opp_eval_expr(expr->right, &right);

			switch (left.obj_type)
			{
				case OBJ_INT:
					if (right.obj_type == OBJ_INT) {
						obj->oint = left.oint + right.oint;
						obj->obj_type = OBJ_INT;
					}
					else if (right.obj_type == OBJ_FLOAT) {
						obj->oint = left.oint + (int)right.ofloat;
						obj->obj_type = OBJ_INT;
					}
					break;

				case OBJ_FLOAT:
					if (right.obj_type == OBJ_INT) {
						obj->ofloat = left.ofloat + right.oint;
						obj->obj_type = OBJ_FLOAT;
					}
					else if (right.obj_type == OBJ_FLOAT) {
						obj->ofloat = left.ofloat + right.ofloat;
						obj->obj_type = OBJ_FLOAT;
					}
					break;

				default:
					opp_error(NULL, "Attempting to add invalid types...");
			}
			break;
		}

		case TMIN: {
			opp_eval_expr(expr->left, &left);
			opp_eval_expr(expr->right,&right);

			switch (left.obj_type)
			{
				case OBJ_INT:
					if (right.obj_type == OBJ_INT) {
						obj->oint = left.oint - right.oint;
						obj->obj_type = OBJ_INT;
					}
					else if (right.obj_type == OBJ_FLOAT) {
						obj->oint = left.oint - (int)right.ofloat;
						obj->obj_type = OBJ_INT;
					}
					break;

				case OBJ_FLOAT:
					if (right.obj_type == OBJ_INT) {
						obj->ofloat = left.ofloat - right.oint;
						obj->obj_type = OBJ_FLOAT;
					}
					else if (right.obj_type == OBJ_FLOAT) {
						obj->ofloat = left.ofloat - right.ofloat;
						obj->obj_type = OBJ_FLOAT;
					}
					break;

				default:
					opp_error(NULL, "Attempting to add invalid types...");
			}
			break;
		}

		case TMUL: {
			opp_eval_expr(expr->left, &left);
			opp_eval_expr(expr->right, &right);

			switch (left.obj_type)
			{
				case OBJ_INT:
					if (right.obj_type == OBJ_INT) {
						obj->oint = left.oint * right.oint;
						obj->obj_type = OBJ_INT;
					}
					else if (right.obj_type == OBJ_FLOAT) {
						obj->oint = left.oint * (int)right.ofloat;
						obj->obj_type = OBJ_INT;
					}
					break;

				case OBJ_FLOAT:
					if (right.obj_type == OBJ_INT) {
						obj->ofloat = left.ofloat * right.oint;
						obj->obj_type = OBJ_FLOAT;
					}
					else if (right.obj_type == OBJ_FLOAT) {
						obj->ofloat = left.ofloat * right.ofloat;
						obj->obj_type = OBJ_FLOAT;
					}
					break;

				default:
					opp_error(NULL, "Attempting to add invalid types...");
			}
			break;
		}

		case TDIV: {
			opp_eval_expr(expr->left, &left);
			opp_eval_expr(expr->right, &right);

			switch (left.obj_type)
			{
				case OBJ_INT:
					if (right.obj_type == OBJ_INT) {
						obj->oint = left.oint / right.oint;
						obj->obj_type = OBJ_INT;
					}
					else if (right.obj_type == OBJ_FLOAT) {
						obj->oint = left.oint / (int)right.ofloat;
						obj->obj_type = OBJ_INT;
					}
					break;

				case OBJ_FLOAT:
					if (right.obj_type == OBJ_INT) {
						obj->ofloat = left.ofloat / right.oint;
						obj->obj_type = OBJ_FLOAT;
					}
					else if (right.obj_type == OBJ_FLOAT) {
						obj->ofloat = left.ofloat / right.ofloat;
						obj->obj_type = OBJ_FLOAT;
					}
					break;

				default:
					opp_error(NULL, "Attempting to add invalid types...");
			}
			break;
		}

		case TMOD: {
			opp_eval_expr(expr->left, &left);
			opp_eval_expr(expr->right, &right);

			obj->obj_type = OBJ_INT;
			switch (left.obj_type)
			{
				case OBJ_INT:
					if (right.obj_type == OBJ_INT)
						obj->oint = left.oint % right.oint;
					else if (right.obj_type == OBJ_FLOAT)
						obj->oint = left.oint % (int)right.ofloat;
					break;

				default:
					opp_error(NULL, "Attempting to mod invalid types...");
			}
		}
	}
}

void opp_eval_logic(struct Opp_Expr_Logic* expr, struct Opp_Obj* obj)
{
	struct Opp_Obj left = {0};
	struct Opp_Obj right = {0};

	switch (expr->tok)
	{
		case TOR: {
			opp_eval_expr(expr->left, &left);
			opp_eval_expr(expr->right, &right);

			if (left.obj_type != OBJ_BOOL && right.obj_type != OBJ_BOOL)
				opp_error(NULL, "Cannot do || on non booleans");
			obj->obool = (left.obool || right.obool);
			obj->obj_type = OBJ_BOOL;
			break;
		}

		case TAND: {
			opp_eval_expr(expr->left,&left);
			opp_eval_expr(expr->right,&right);

			if (left.obj_type != OBJ_BOOL && right.obj_type != OBJ_BOOL)
				opp_error(NULL, "Cannot preform opertaion &&");
			obj->obool = (left.obool && right.obool);
			obj->obj_type = OBJ_BOOL;
			break;
		}

		case TEQEQ: {
			opp_eval_expr(expr->left,&left);
			opp_eval_expr(expr->right,&right);

			if (left.obj_type == OBJ_INT && right.obj_type == OBJ_INT)
				obj->obool = (left.oint == right.oint);

			else if (left.obj_type == OBJ_FLOAT && right.obj_type == OBJ_FLOAT) {
				if (left.ofloat == right.ofloat)
					obj->obool = 1;
				else 
					obj->obool = 0; 
			}

			else if (left.obj_type == OBJ_BOOL && right.obj_type == OBJ_BOOL)
				obj->obool = (left.obool == right.obool);

			else if (left.obj_type == OBJ_STR && right.obj_type == OBJ_STR)
				obj->obool = !strcmp(left.ostr, right.ostr);

			else if (left.obj_type != right.obj_type) {
				printf("Mixing types in '==' expression causing false\n");
				obj->obool = 0;
			}
			obj->obj_type = OBJ_BOOL;
			break;
		}

		case TGT: {
			opp_eval_expr(expr->left,&left);
			opp_eval_expr(expr->right,&right);

			if (left.obj_type == OBJ_INT && right.obj_type == OBJ_INT)
				obj->obool = (left.oint > right.oint);

			else if (left.obj_type == OBJ_FLOAT && right.obj_type == OBJ_FLOAT) {
				if (left.ofloat > right.ofloat)
					obj->obool = 1;
				else 
					obj->obool = 0; 
			}

			else if (left.obj_type == OBJ_BOOL && right.obj_type == OBJ_BOOL)
				obj->obool = (left.obool > right.obool);

			else if (left.obj_type != right.obj_type) {
				printf("Mixing types in '>' expression causing false\n");
				obj->obool = 0;
			}
			obj->obj_type = OBJ_BOOL;
			break;
		}

		case TLT: {
			opp_eval_expr(expr->left,&left);
			opp_eval_expr(expr->right,&right);

			if (left.obj_type == OBJ_INT && right.obj_type == OBJ_INT)
				obj->obool = (left.oint < right.oint);

			else if (left.obj_type == OBJ_FLOAT && right.obj_type == OBJ_FLOAT) {
				if (left.ofloat < right.ofloat)
					obj->obool = 1;
				else 
					obj->obool = 0; 
			}

			else if (left.obj_type == OBJ_BOOL && right.obj_type == OBJ_BOOL)
				obj->obool = (left.obool < right.obool);

			else if (left.obj_type != right.obj_type) {
				printf("Mixing types in '<' expression causing false\n");
				obj->obool = 0;
			}
			obj->obj_type = OBJ_BOOL;
			break;
		}

		default:
			opp_error(NULL, "Unknown logic operator");

	}
}

// void opp_eval_array(struct Opp_Expr_Array* expr, struct Opp_Obj* obj)
// {
// 	obj->obj_type = OBJ_ARRAY;
// 	obj->arr.size = expr->amount;

// 	struct Opp_Obj temp = {0};
// 	opp_eval_expr(expr->elements[0], &temp);

// 	obj->arr.array_type = temp.obj_type;

// 	for (int i = 0; i < expr->amount; i++)
// 	{
// 		if (i > 0) {
// 			opp_eval_expr(expr->elements[i], &temp);

// 			if (temp.obj_type != obj->arr.array_type)
// 				opp_error(NULL, "Attempt to mix types in array");
// 		}

// 		switch (obj->arr.array_type) 
// 		{
// 			case OBJ_INT: obj->arr.iarr[i] = temp.oint; break;
// 			case OBJ_FLOAT: obj->arr.darr[i] = temp.ofloat; break;
// 			case OBJ_BOOL: obj->arr.barr[i] = temp.obool; break;
// 			case OBJ_STR: strcpy(obj->arr.sarr[i], temp.ostr); break;
// 		}
// 	}
// }

void opp_eval_call(struct Opp_Expr_Call* expr, struct Opp_Obj* obj)
{
	struct Opp_Expr* in = (struct Opp_Expr*)(expr->callee);

	if (in->type != EUNARY)
		opp_error(NULL, "Expected identifier for func call");

	struct Opp_Expr_Unary* unary = (struct Opp_Expr_Unary*)(in->expr);

	if (unary->val.strval == NULL)
		opp_error(NULL, "Expected identifier for func call");
	
	int func_type = -1;
	unsigned int loc = hash_str(unary->val.strval, current_ns->inside);
	struct Namespace* search = current_ns;

	while (search != NULL)
	{
		if (search->inside->list[loc] != NULL) {
			func_type = env_get_type(search->inside, unary->val.strval);
			break;
		}
		search = search->parent;
	}

	if (func_type == -1)
		opp_error(NULL, "Undefined func call '%s'", unary->val.strval);

	if (func_type == VCFUNC)
	{
		void (*func)(struct Opp_List* args, struct Opp_Obj* obj) = env_get_cfn(search->inside, unary->val.strval);
		func(expr->args, obj);
		return;
	}
	else if (func_type == VFUNC)
	{
		struct Hash_Node* fn = env_get_fn(search->inside, unary->val.strval);

		// fn->func->local = init_namespace(unary->val.strval, current_ns);

		env_add_local(fn->func->local->inside, unary->val.strval, expr->args, fn->func->arg_name);

		struct Opp_Obj need;
		struct Opp_Stmt_Block* block = (struct Opp_Stmt_Block*)(fn->func->stmts->stmt); 
		struct Namespace* temp = current_ns;
		current_ns = fn->func->local;

		int i = 0;
		while (block->stmts[i] != NULL && opp_state.trigger_ret == 0)
		{
			opp_eval(block->stmts[i], &need);
			i++;
		}
		current_ns = temp;

		for (int a = 0; a < fn->func->local->inside->size; a++)
			fn->func->local->inside->list[a] = NULL;
		memset(fn->func->local, sizeof(fn->func->local), 0);

		// for (int a=0; a<fn->func->local->inside->size; a++)
		// 	free(fn->func->local->inside->list[a]);
		// free(fn->func->local->inside);
		// free(fn->func->local);

		if (opp_state.trigger_ret == 1)
		{
			obj->obj_type = opp_state.val.obj_type;
			switch (opp_state.val.obj_type)
			{
				case OBJ_INT: obj->oint = opp_state.val.oint; break;

				case OBJ_BOOL: obj->obool = opp_state.val.obool; break;

				case OBJ_STR: strcpy(obj->ostr, opp_state.val.ostr); break;

				case OBJ_FLOAT: obj->ofloat = opp_state.val.ofloat; break;
			}
			opp_state.trigger_ret = 0;
			memset(&opp_state.val, sizeof(opp_state.val), 0);
		}
	}

}

void opp_eval_ifstmt(struct Opp_Stmt_If* expr, struct Opp_Obj* obj)
{
	obj->obj_type = OBJ_NONE;
	struct Opp_Obj result = {0};
	struct Opp_Obj temp = {0};
	opp_eval_expr(expr->cond, &result);

	if (result.obj_type != OBJ_BOOL)
		opp_error(NULL, "Expected expression in if statement");

	if (result.obool == 1)
		opp_eval(expr->then, &temp);
	else if (result.obool == 0 && expr->other != NULL)
		opp_eval(expr->other, &temp);

}

void opp_eval_block(struct Opp_Stmt_Block* expr, struct Opp_Obj* obj)
{
	obj->obj_type = OBJ_NONE;
	struct Opp_Obj need = {0};
	struct Namespace* temp = current_ns;
	struct Namespace* new_ns = init_namespace("block", temp);
	current_ns = new_ns;

	int i = 0;
	while (expr->stmts[i] != NULL)
	{
		opp_eval(expr->stmts[i], &need);
		i++;
	}

	for (int a=0; a<new_ns->inside->size; a++)
		free(new_ns->inside->list[a]);
	free(new_ns->inside->list);
	free(new_ns->inside);
	free(new_ns);

	current_ns = temp;
	
}

void opp_eval_assign(struct Opp_Expr_Assign* expr, struct Opp_Obj* obj)
{
	obj->obj_type = OBJ_NONE;
	struct Opp_Expr* a = (struct Opp_Expr*)(expr->ident);
	struct Opp_Expr* value = (struct Opp_Expr*)(expr->val);

	if (a->type != EUNARY)
		opp_error(NULL, "Expected identifier in assigment");

	struct Opp_Expr_Unary* b = (struct Opp_Expr_Unary*)(a->expr);
	struct Opp_Obj final_val = {0};
	opp_eval_expr(value, &final_val);

	if (b->type != IDENT)
		opp_error(NULL, "Expected identifier in assigment");

	unsigned int loc = hash_str(b->val.strval, current_ns->inside);
	struct Namespace* search = current_ns;

	int type = -1;
	while (search != NULL)
	{
		if (search->inside->list[loc] != NULL) {
			type = env_get_type(search->inside, b->val.strval);
			break;
		}
		search = search->parent;
	}
	// Collison with parent namespace
	// printf("==>%s %d %d\n", search->inside->list[loc]->key, hash_str("j", current_ns->inside), hash_str("ball_x", current_ns->inside));

	if (type == -1)
		opp_error(NULL, "Undefined identifier in assign '%s'", b->val.strval);

	switch (expr->op)
	{
		case TEQ: {
			if (type == VINT && final_val.obj_type == OBJ_INT)
				env_new_int(search->inside, b->val.strval, final_val.oint);
			else if (type == VDOUBLE && final_val.obj_type == OBJ_FLOAT)
				env_new_dbl(search->inside, b->val.strval, final_val.ofloat);
			else if (type == VSTR && final_val.obj_type == OBJ_STR)
				env_new_str(search->inside, b->val.strval, final_val.ostr);
			else if (type == VBOOL && final_val.obj_type == OBJ_BOOL)
				env_new_bool(search->inside, b->val.strval, final_val.obool);
			else if (type == VNONE) 
			{
				if (final_val.obj_type == OBJ_INT) {
					search->inside->list[loc]->type = VINT;
					env_new_int(search->inside, b->val.strval, final_val.oint);
				}
				else if (final_val.obj_type == OBJ_FLOAT) {
					search->inside->list[loc]->type = VDOUBLE;
					env_new_dbl(search->inside, b->val.strval, final_val.ofloat);
				}
				else if (final_val.obj_type == OBJ_STR) {
					search->inside->list[loc]->type = VSTR;
					env_new_str(search->inside, b->val.strval, final_val.ostr);
				}
				else if (final_val.obj_type == OBJ_BOOL) {
					search->inside->list[loc]->type = VBOOL;
					env_new_bool(search->inside, b->val.strval, final_val.obool);
				}
			}

			else 
				opp_error(NULL, "Attempt to switch var type in assign '%s'", b->val.strval);
		}
	}
}

void opp_eval_var(struct Opp_Stmt_Var* expr, struct Opp_Obj* obj)
{
	obj->obj_type = OBJ_NONE;
	struct Opp_Expr* a = (struct Opp_Expr*)(expr->ident);

	if (a->type == EUNARY)
	{
		struct Opp_Expr_Unary* name = (struct Opp_Expr_Unary*)(a->expr);
		int type = env_get_type(current_ns->inside, name->val.strval);

		if (type != -1)
			opp_error(NULL, "Attempt to change var '%s' to type none", name->val.strval);

		env_new_none(current_ns->inside, name->val.strval);
		return;
	}

	if (a->type != EASSIGN)
		opp_error(NULL, "Expected expression to assign a variable var <ident> '=' <expr>");

	struct Opp_Expr_Assign* b = (struct Opp_Expr_Assign*)(a->expr);
	struct Opp_Expr* c = (struct Opp_Expr*)(b->val);
	struct Opp_Expr* d = (struct Opp_Expr*)(b->ident);
	struct Opp_Expr_Unary* e = (struct Opp_Expr_Unary*)(d->expr);
	struct Opp_Obj new_val = {0};
	opp_eval_expr(c, &new_val);

	int type = env_get_type(current_ns->inside, e->val.strval);
	int new_type = new_val.obj_type;

	if (
	(type == VINT && new_type != OBJ_INT) ||
	(type == VDOUBLE && new_type != OBJ_FLOAT) ||
	(type == VBOOL && new_type != OBJ_BOOL) ||
	(type == VSTR && new_type != OBJ_STR))
		opp_error(NULL, "Attempt to redeclare value '%s'", e->val.strval);

	switch (new_val.obj_type)
	{
		case OBJ_INT:
			env_new_int(current_ns->inside, e->val.strval, new_val.oint);
			break;

		case OBJ_STR:
			env_new_str(current_ns->inside, e->val.strval, new_val.ostr);
			break;

		case OBJ_FLOAT:
			env_new_dbl(current_ns->inside, e->val.strval, new_val.ofloat);
			break;

		case OBJ_BOOL:
			env_new_bool(current_ns->inside, e->val.strval, new_val.obool);
			break;

		// case OBJ_ARRAY:
		// 	env_new_array(current_ns->inside, e->val.strval, &new_val.arr);
		// 	break;

		default:
			opp_error(NULL, "Error to assign a value to '%s'", e->val.strval);

	}
	
}

void opp_eval_import(struct Opp_Stmt_Import* expr, struct Opp_Obj* obj)
{
	obj->obj_type = OBJ_NONE;
	struct Opp_Expr* a = (struct Opp_Expr*)(expr->ident);

	if (a->type != EUNARY)
		opp_error(NULL, "Expected string after import statement");

	struct Opp_Obj str = {0};
	opp_eval_expr(a, &str);

	if (str.obj_type != OBJ_STR)
		opp_error(NULL, "Expected string after import statement");

	// Init Modules
	// if (!strcmp(str.ostr, "raylib")) init_raylib();

	else init_opp(str.ostr);

}

void opp_eval_while(struct Opp_Stmt_While* expr, struct Opp_Obj* obj)
{
	obj->obj_type = OBJ_NONE;
	struct Opp_Expr* a = (struct Opp_Expr*)(expr->cond);
	struct Opp_Obj res = {0};
	opp_eval_expr(a, &res);
	struct Opp_Stmt* b = (struct Opp_Stmt*)(expr->then);
	struct Opp_Stmt_Block* block = NULL;
	struct Opp_Obj need;

	if (res.obj_type != OBJ_BOOL)
		opp_error(NULL, "Expected a condition statement in while loop");

	if (b->type != STMT_BLOCK)
		opp_error(NULL, "Expected block after while loop");

	block = (struct Opp_Stmt_Block*)(b->stmt);

	struct Namespace* temp = current_ns; 
	struct Namespace* new_ns = init_namespace("block", current_ns);

	current_ns = new_ns;

	int i = 0;
	while (res.obool == 1)
	{
		while (block->stmts[i] != NULL) {
			opp_eval(block->stmts[i], &need);
			i++;
		}
		i = 0;
		opp_eval_expr(a, &res);
	}

	current_ns = temp;

	for (int a=0; a<new_ns->inside->size; a++)
		free(new_ns->inside->list[a]);	
	free(new_ns->inside);
	free(new_ns);
}

void opp_eval_func(struct Opp_Stmt_Func* expr, struct Opp_Obj* obj)
{
	obj->obj_type = OBJ_NONE;
	struct Opp_Expr* a = (struct Opp_Expr*)(expr->name);
	struct Opp_Expr_Unary* b = (struct Opp_Expr_Unary*)(a->expr);

	struct Opp_Stmt* body = (struct Opp_Stmt*)(expr->body);
	struct Opp_List* args = (struct Opp_List*)(expr->args);

	if (env_get_type(current_ns->inside, b->val.strval) != -1)
		opp_error(NULL, "Function '%s' already has been defined", b->val.strval);

	env_new_fn(current_ns->inside, b->val.strval, body, args);
}

void opp_eval_return(struct Opp_Stmt_Ret* expr, struct Opp_Obj* obj)
{
	obj->obj_type = OBJ_NONE;

	opp_eval_expr(expr->value, &opp_state.val);
	opp_state.trigger_ret = 1;
}