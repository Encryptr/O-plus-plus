#include "interpreter.h"

#define DEBUG

void opp_eval_init(struct Opp_Parser* parser)
{
	if (global_ns == NULL || parser == NULL)
		return;

	for (int stmt=0; stmt<parser->nstmt; stmt++)
	{
		struct Opp_Obj* ret_val = opp_eval(parser->statments[stmt]);

		if (parser->mode == IREPL)
			opp_repl_ret(ret_val);
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

		case STMT_IMPORT: {
			struct Opp_Stmt_Import* temp = (struct Opp_Stmt_Import*)(stmt->stmt);
			return opp_eval_import(temp);
		}

		case STMT_WHILE: {
			struct Opp_Stmt_While* temp = (struct Opp_Stmt_While*)(stmt->stmt);
			return opp_eval_while(temp);
		}

		case STMT_FUNC: {
			struct Opp_Stmt_Func* temp = (struct Opp_Stmt_Func*)(stmt->stmt);
			return opp_eval_func(temp);
		}

		case STMT_RET: {
			struct Opp_Stmt_Ret* temp = (struct Opp_Stmt_Ret*)(stmt->stmt);
			return opp_eval_return(temp);
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

								case VDOUBLE:
									literal = obj_make(OBJ_FLOAT);
									literal->ofloat = search->inside->list[loc]->value.dval;
									break;

								case VBOOL:
									literal = obj_make(OBJ_BOOL);
									literal->obool = search->inside->list[loc]->value.bval;
									break;

								case VSTR:
									literal = obj_make(OBJ_STR);
									literal->ostr = (char*)malloc(
										sizeof(strlen(search->inside->list[loc]->value.strval)));
									strcpy(literal->ostr, search->inside->list[loc]->value.strval);
									break;

							}
							return literal;
						}
						search = search->parent;
					}
					opp_error(NULL, "Identifier '%s' not declared", temp->val.strval);
				}

				case STR:
					literal = obj_make(OBJ_STR);
					literal->ostr = (char*)malloc(strlen(temp->val.strval));
					strcpy(literal->ostr, temp->val.strval);
					return literal;
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

		case ESUB: {
			struct Opp_Expr_Sub* temp = (struct Opp_Expr_Sub*)(expr->expr);
			return opp_eval_sub(temp);
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

struct Opp_Obj* opp_eval_sub(struct Opp_Expr_Sub* expr)
{
	struct Opp_Obj* val = opp_eval_expr(expr->unary);

	switch (val->obj_type) 
	{
		case OBJ_INT: val->oint *= -1; break;
		case OBJ_FLOAT: val->ofloat *= -1.0; break;

		default:
			opp_error(NULL, "Error attempting to negate invalid type");
	}

	return val;
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

		case TGT: {
			left = opp_eval_expr(expr->left);
			right = opp_eval_expr(expr->right);

			if (left->obj_type == OBJ_INT && right->obj_type == OBJ_INT)
				left->obool = (left->oint > right->oint);

			else if (left->obj_type == OBJ_FLOAT && right->obj_type == OBJ_FLOAT) {
				if (left->ofloat > right->ofloat)
					left->obool = 1;
				else 
					left->obool = 0; 
			}

			else if (left->obj_type == OBJ_BOOL && right->obj_type == OBJ_BOOL)
				left->obool = (left->obool > right->obool);

			else if (left->obj_type != right->obj_type) {
				printf("Mixing types in '>' expression causing false\n");
				left->obool = 0;
			}
			left->obj_type = OBJ_BOOL;

			return left;
		}

		case TLT: {
			left = opp_eval_expr(expr->left);
			right = opp_eval_expr(expr->right);

			if (left->obj_type == OBJ_INT && right->obj_type == OBJ_INT)
				left->obool = (left->oint < right->oint);

			else if (left->obj_type == OBJ_FLOAT && right->obj_type == OBJ_FLOAT) {
				if (left->ofloat < right->ofloat)
					left->obool = 1;
				else 
					left->obool = 0; 
			}

			else if (left->obj_type == OBJ_BOOL && right->obj_type == OBJ_BOOL)
				left->obool = (left->obool < right->obool);

			else if (left->obj_type != right->obj_type) {
				printf("Mixing types in '<' expression causing false\n");
				left->obool = 0;
			}
			left->obj_type = OBJ_BOOL;

			return left;

		}

		default:
			opp_error(NULL, "Unknown logic operator");

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
		void (*func)(struct Opp_List* args) = env_get_cfn(search->inside, unary->val.strval);
		func(expr->args);
	}
	else if (func_type == VFUNC)
	{
		struct Hash_Node* fn = env_get_fn(search->inside, unary->val.strval);

		fn->func->local = init_namespace(unary->val.strval, current_ns);

		env_add_local(fn->func->local->inside, unary->val.strval, expr->args, fn->func->arg_name);

		struct Opp_Stmt_Block* block = (struct Opp_Stmt_Block*)(fn->func->stmts->stmt); 
		struct Namespace* temp = current_ns;
		current_ns = fn->func->local;

		int i = 0;
		while (block->stmts[i] != NULL && opp_state.trigger_ret == 0)
		{
			opp_eval(block->stmts[i]);
			i++;
		}
		current_ns = temp;

		for (int a=0; a<fn->func->local->inside->size; a++)
			free(fn->func->local->inside->list[a]);
		free(fn->func->local->inside);
		free(fn->func->local);

		if (opp_state.trigger_ret == 1)
		{
			obj = obj_make(opp_state.val->obj_type);

			switch (opp_state.val->obj_type)
			{
				case OBJ_INT: obj->oint = opp_state.val->oint; break;

				case OBJ_BOOL: obj->obool = opp_state.val->obool; break;

				case OBJ_STR: obj->ostr = opp_state.val->ostr; break;

				case OBJ_FLOAT: obj->ofloat = opp_state.val->ofloat; break;
			}
			opp_state.trigger_ret = 0;
			opp_state.val = NULL;

			return obj;
		}
	}

	obj = obj_make(OBJ_NONE);

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

	int type = -1;
	while (search != NULL)
	{
		if (search->inside->list[loc] != NULL) {
			type = env_get_type(search->inside, b->val.strval);
			break;
		}
		search = search->parent;
	}

	if (type == -1)
		opp_error(NULL, "Undefined identifier in assign '%s'", b->val.strval);

	switch (expr->op)
	{
		case TEQ: {
			if (type == VINT && final_val->obj_type == OBJ_INT)
				env_new_int(search->inside, b->val.strval, final_val->oint);
			else if (type == VDOUBLE && final_val->obj_type == OBJ_FLOAT)
				env_new_dbl(search->inside, b->val.strval, final_val->ofloat);
			else if (type == VSTR && final_val->obj_type == OBJ_STR)
				env_new_str(search->inside, b->val.strval, final_val->ostr);
			else if (type == VBOOL && final_val->obj_type == OBJ_BOOL)
				env_new_bool(search->inside, b->val.strval, final_val->obool);
			else 
				opp_error(NULL, "Attempt to switch var type in assign '%s'", b->val.strval);
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

	int type = env_get_type(current_ns->inside, e->val.strval);
	int new_type = new_val->obj_type;

	if (
	(type == VINT && new_type != OBJ_INT) ||
	(type == VDOUBLE && new_type != OBJ_FLOAT) ||
	(type == VBOOL && new_type != OBJ_BOOL) ||
	(type == VSTR && new_type != OBJ_STR))
		opp_error(NULL, "Attempt to redeclare value '%s'", e->val.strval);

	switch (new_val->obj_type)
	{
		case OBJ_INT:
			env_new_int(current_ns->inside, e->val.strval, new_val->oint);
			break;

		case OBJ_STR:
			env_new_str(current_ns->inside, e->val.strval, new_val->ostr);
			break;

		case OBJ_FLOAT:
			env_new_dbl(current_ns->inside, e->val.strval, new_val->ofloat);
			break;

		case OBJ_BOOL:
			env_new_bool(current_ns->inside, e->val.strval, new_val->obool);
			break;

		default:
			opp_error(NULL, "Error to assign a value to '%s'", e->val.strval);

	}
	
	return none;
}

struct Opp_Obj* opp_eval_import(struct Opp_Stmt_Import* expr)
{
	struct Opp_Obj* none = obj_make(OBJ_NONE);
	struct Opp_Expr* a = (struct Opp_Expr*)(expr->ident);

	if (a->type != EUNARY)
		opp_error(NULL, "Expected string after import statement");

	struct Opp_Obj* str = opp_eval_expr(a);

	if (str->obj_type != OBJ_STR && str->ostr != NULL)
		opp_error(NULL, "Expected string after import statement");

	// struct Namespace* new_file = init_namespace(str->ostr, NULL);
	// struct Namespace* temp = current_ns;
	// current_ns = new_file;

	init_opp(str->ostr);

	// current_ns = temp;

	return none;
}

struct Opp_Obj* opp_eval_while(struct Opp_Stmt_While* expr)
{
	struct Opp_Obj* none = obj_make(OBJ_NONE);
	struct Opp_Expr* a = (struct Opp_Expr*)(expr->cond);
	struct Opp_Obj* res = opp_eval_expr(a);
	struct Opp_Stmt* b = (struct Opp_Stmt*)(expr->then);
	struct Opp_Stmt_Block* block = NULL;

	if (res->obj_type != OBJ_BOOL)
		opp_error(NULL, "Expected a condition statement in while loop");

	if (b->type != STMT_BLOCK)
		opp_error(NULL, "Expected block after while loop");

	block = (struct Opp_Stmt_Block*)(b->stmt);

	struct Namespace* temp = current_ns;
	struct Namespace* new_ns = init_namespace("block", temp);

	current_ns = new_ns;

	int i = 0;
	while (res->obool == 1)
	{
		while (block->stmts[i] != NULL) {
			opp_eval(block->stmts[i]);
			i++;
		}
		i = 0;
		res = opp_eval_expr(a);
	}

	current_ns = temp;

	for (int a=0; a<new_ns->inside->size; a++)
		free(new_ns->inside->list[a]);

	free(new_ns->inside);
	free(new_ns);


	return none;
}

struct Opp_Obj* opp_eval_func(struct Opp_Stmt_Func* expr)
{
	struct Opp_Obj* none = obj_make(OBJ_NONE);
	struct Opp_Expr* a = (struct Opp_Expr*)(expr->name);
	struct Opp_Expr_Unary* b = (struct Opp_Expr_Unary*)(a->expr);

	struct Opp_Stmt* body = (struct Opp_Stmt*)(expr->body);
	struct Opp_List* args = (struct Opp_List*)(expr->args);

	if (env_get_type(current_ns->inside, b->val.strval) != -1)
		opp_error(NULL, "Function '%s' already has been defined", b->val.strval);

	env_new_fn(current_ns->inside, b->val.strval, body, args);

	return none;
}

struct Opp_Obj* opp_eval_return(struct Opp_Stmt_Ret* expr)
{
	struct Opp_Obj* none = obj_make(OBJ_NONE);	

	opp_state.val = opp_eval_expr(expr->value);
	opp_state.trigger_ret = 1;

	return none;
}
