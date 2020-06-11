#include "ostdlib.h"

void opp_init_std()
{
	// NULL Value
	env_new_int(global_ns->inside, "NULL", 0);

	// Std types
	env_new_int(global_ns->inside, "INT", 1);
	env_new_int(global_ns->inside, "REAL", 2);
	env_new_int(global_ns->inside, "BOOL", 3);
	env_new_int(global_ns->inside, "STRING", 4);

	if (!env_new_cfn(global_ns->inside, "echo", echo))
		internal_error("STD Fail", 2);

	if (!env_new_cfn(global_ns->inside, "print", opp_print))
		internal_error("STD Fail", 2);

	if (!env_new_cfn(global_ns->inside, "input", opp_input))
		internal_error("STD Fail", 2);

	if (!env_new_cfn(global_ns->inside, "rand", opp_rand))
		internal_error("STD Fail", 2);

	if (!env_new_cfn(global_ns->inside, "typeof", opp_typeof))
		internal_error("STD Fail", 2);

	if (!env_new_cfn(global_ns->inside, "append", opp_append))
		internal_error("STD Fail", 2);

	if (!env_new_cfn(global_ns->inside, "getc", opp_getc))
		internal_error("STD Fail", 2);
}

void echo(struct Opp_List* args, struct Opp_Obj* obj)
{
	obj->obj_type = OBJ_NONE;
	for (int i=0; i<args->size; i++)
	{
		struct Opp_Obj res = {0};
		opp_eval_expr(args->list[i], &res);
		
		switch (res.obj_type)
		{
			case OBJ_INT:
				printf("%d ", res.oint);
				break;

			case OBJ_FLOAT:
				printf("%.1lf ", res.ofloat);
				break;

			case OBJ_STR:
				printf("%s ", res.ostr);
				break;

			case OBJ_BOOL:
				printf("%d ", res.obool);
				break;
		}
	}
	printf("\n");
}

void opp_print(struct Opp_List* args, struct Opp_Obj* obj)
{
	obj->obj_type = OBJ_NONE;
	for (int i=0; i<args->size; i++)
	{
		struct Opp_Obj res = {0};
		opp_eval_expr(args->list[i], &res);
		switch (res.obj_type)
		{
			case OBJ_INT:
				printf("%d\n", res.oint);
				break;

			case OBJ_FLOAT:
				printf("%.3lf\n", res.ofloat);
				break;

			case OBJ_STR:
				printf("%s\n", res.ostr);
				break;

			case OBJ_BOOL:
				printf("%d\n", res.obool);
				break;
		}
	}
}

void opp_append(struct Opp_List* args, struct Opp_Obj* obj)
{
	obj->obj_type = OBJ_INT;

	expect_args(2);

	struct Opp_Expr_Unary* name = (struct Opp_Expr_Unary*)(args->list[0]->expr);

	if (name->type != IDENT)
		opp_error(NULL, "Expected array as first argument in func 'append'");

	unsigned int loc = hash_str(name->val.strval, current_ns->inside);
	struct Namespace* search = current_ns;

	int type = -1;
	while (search != NULL)
	{
		if (search->inside->list[loc] != NULL) {

			struct Opp_Obj res = {0};
			opp_eval_expr(args->list[1], &res);
			env_new_element(search->inside, name->val.strval, &res);
			obj->oint = 1;
			return;
		}
		search = search->parent;
	}
	obj->oint = 0;
}

void opp_getc(struct Opp_List* args, struct Opp_Obj* obj)
{
	obj->obj_type = OBJ_STR;

	char out[2] = {0};
	out[0] = getchar();
	strcpy(obj->ostr, out);
}

void opp_typeof(struct Opp_List* args, struct Opp_Obj* obj)
{
	obj->obj_type = OBJ_INT;
	expect_args(1);

	struct Opp_Obj type = {0};
	opp_eval_expr(args->list[0], &type);

	switch (type.obj_type)
	{
		case OBJ_INT:   obj->oint = 1; break;
		case OBJ_FLOAT: obj->oint = 2; break;
		case OBJ_BOOL:  obj->oint = 3; break; 
		case OBJ_STR:   obj->oint = 4; break;
	}
}

void opp_input(struct Opp_List* args, struct Opp_Obj* obj)
{
	expect_args(0);

	char input[255] = {0};
	scanf("%s", input);

	obj->obj_type = OBJ_STR;
	strcpy(obj->ostr, input);
}

void opp_rand(struct Opp_List* args, struct Opp_Obj* obj)
{
	expect_args(0);
	obj->obj_type = OBJ_INT;
	obj->oint = rand();
}