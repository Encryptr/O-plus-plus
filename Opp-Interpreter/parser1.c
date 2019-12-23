#include "parser.h"

void expect_semi(struct Scan* s)
{
	if (s->tok != SEMICOLON)
		printf("[%ld] Expected ';'\n", s->line), exit(1);
}

struct Opp_Value opp_parse_type(struct Scan* s, struct Table* local)
{
	struct Opp_Value value = {0};
	
	if (s->tok == IDENT)
	{
		struct Opp_Value temp = {0};

		temp = opp_parse_varstr(s, local);
		if (temp.val_type != T_INTEGER)
		{
			temp.val_type = T_STRING;
			return temp;
		}
	}
	if (s->tok == NUM || s->tok == IDENT)
	{
		value = opp_expr_one(s, local);
		value.val_type = T_INTEGER;
	}
	else if (s->tok == TIK)
	{
		value = opp_parse_str(s, local);
		value.val_type = T_STRING;
	}
	else value.val_type = T_INVALID;

	return value;
}

struct Opp_Value opp_expr_one(struct Scan* s, struct Table* local)
{
	// next(s);
	struct Opp_Value right = {0};
	struct Opp_Value left  = {0};
	int operator = 0;

	left = opp_expr_two(s, local);
	operator = s->tok;

	while (operator == PLUS || operator == MINUS)
	{
		next(s);
		right = opp_expr_two(s, local);

		if (operator == PLUS)
			left.ival += right.ival;
		else if (operator == MINUS)
			left.ival -= right.ival;
		operator = s->tok;
	}
	return left;
}

struct Opp_Value opp_expr_two(struct Scan* s, struct Table* local)
{
	struct Opp_Value right = {0};
	struct Opp_Value left  = {0};
	int operator = 0;

	opp_parse_num(s, &left, local);
	next(s);
	operator = s->tok;

	while (operator == DIVIDE || operator == MULTI)
	{
		next(s);
		opp_parse_num(s, &right, local);

		if (operator == DIVIDE)
			left.ival /= right.ival;
		else if (operator == MULTI)
			left.ival *= right.ival;
		next(s);
		operator = s->tok;
	}
	return left;
}

void opp_parse_num(struct Scan* s, struct Opp_Value* value, struct Table* local)
{
	if (s->tok == NUM)
		value->ival = atoi(s->lexeme);
	else if (s->tok == IDENT)
	{
		unsigned int loc = hash_str(s->lexeme);

		if (s->block > 0 && local->list[loc] != NULL)
		{
			if (local->list[loc]->type != INT)
				printf("[%ld] Local variable '%s' must be a number\n", s->line, s->lexeme), exit(1);
			value->ival = local->list[loc]->v1;
		}
		else 
		{
			if (map->list[loc] == NULL)
				printf("[%ld] Undeclared identifier '%s'\n", s->line, s->lexeme), exit(1);
			if (map->list[loc]->type != INT)
				printf("[%ld] Variable '%s' must be a number\n", s->line, s->lexeme), exit(1);
			value->ival = map->list[loc]->v1;
		}
	}
}

struct Opp_Value opp_parse_varstr(struct Scan* s, struct Table* local)
{
	struct Opp_Value value = {0};
	unsigned int loc = hash_str(s->lexeme);

	if (s->block > 0 && local->list[loc] != NULL)
	{
		if (local->list[loc]->type != STRING)
			{value.val_type = T_INTEGER; return value;}
		else
		{
			next(s);
			value.strval = malloc(sizeof(char*)*strlen(local->list[loc]->v3)+1);
			strcpy(value.strval, local->list[loc]->v3);
			return value;
		}
	}
	else {
		if (map->list[loc] == NULL)
			printf("[%ld] Undeclared identifier '%s'\n", s->line, s->lexeme), exit(1);
		if (map->list[loc]->type != STRING)
			{value.val_type = T_INTEGER; return value;}
		else
		{
			next(s);
			value.strval = malloc(sizeof(char*)*strlen(map->list[loc]->v3)+1);
			strcpy(value.strval, map->list[loc]->v3);
			return value;
		}
	}
	return value;
}

struct Opp_Value opp_parse_str(struct Scan* s, struct Table* local)
{
	struct Opp_Value str = {0};

	if (all_until('\'', s) == 0)
		{ printf("[%ld] No terminating [']\n", s->line); exit(1);}

	str.strval = malloc(sizeof(char)*(strlen(s->lexeme)+1));
	strcpy(str.strval, s->lexeme);
	next(s);
	next(s);

	return str;
}

void opp_parse_var(struct Scan* s, struct Table* local)
{
	char varname[10];
	varname[0] = '\0';
	struct Opp_Value val = {0};

	next(s);

	if (s->tok != IDENT)
		printf("[%ld] Expected identifier after 'var'\n", s->line), exit(1);
	strcpy(varname, s->lexeme);
	unsigned int loc = hash_str(s->lexeme);

	if (s->block > 0) {
		if (map->list[loc] != NULL || local->list[loc] != NULL)
			printf("[%ld] Variable '%s' already defined\n", s->line, s->lexeme), exit(1);
	}
	else {
		if (map->list[loc] != NULL)
			printf("[%ld] Variable '%s' already defined\n", s->line, s->lexeme), exit(1);
	}
	next(s);

	if (s->tok != EQ)
		printf("[%ld] Expected '=' after 'var'\n", s->line), exit(1);
	next(s);

	val = opp_parse_type(s, local);

	switch (val.val_type)
	{
		case T_INVALID:
			printf("[%ld] Unknown type in '%s' var declaration\n", s->line, varname); 
			exit(1);
		break;

		case T_INTEGER:
			if (s->block > 0) insert_int(local, varname, val.ival);
			else insert_int(map, varname, val.ival);
		break;

		case T_STRING:
			if (s->block > 0) insert_str(local, varname, val.strval);
			else insert_str(map, varname, val.strval);
		break;
	}
	expect_semi(s);
}

void opp_ignore(struct Scan* s)
{
	next(s);
	int loop = 1;

	while (loop > 0)
	{
		if (s->tok == FEND) printf("[%ld] Missing terminating '}'\n", s->line), exit(1);

		else if (s->tok == TIK)
		{
			if (all_until('\'', s) == 0)
				{ printf("[%ld] No terminating [']\n", s->line); exit(1);}
			next(s);
		}
		else if (s->tok == OPENB) loop++;
		else if (s->tok == CLOSEB) loop--;

		if (loop == 0) break;

		next(s);
	}
}

void opp_parse_param(struct Scan* s, unsigned int loc)
{
	int amount = 0;

	char temp_ident[10][11];
	temp_ident[0][0] = '\0';

	if (s->tok != OPENP)
		printf("[%ld] Expected '[' in function declaration\n", s->line), exit(1);
	next(s);

	while (s->tok != CLOSEP) 
	{
		if (s->tok == IDENT)
		{
			strcpy(temp_ident[amount], s->lexeme);
			amount++;
		}	
		else
		{
			if (strlen(s->lexeme) == 0)
				printf("[%ld] Unexpected symbol in parameter declaration\n", s->line), exit(1);
			else printf("[%ld] Unexpected symbol '%s' in parameter declaration\n", s->line, s->lexeme), exit(1);
		}
		next(s);

		if (s->tok == CLOSEP) break;
		else if (s->tok != COMMA)
		{
			if (strlen(s->lexeme) == 0)
				printf("[%ld] Expected ',' in parameter declaration\n", s->line), exit(1);
			else printf("[%ld] Expected ',' in parameter declaration not '%s'\n", s->line, s->lexeme), exit(1);
		}
		next(s);
	}
	map->list[loc]->func.exp_param = amount;
	if (amount != 0)
	{
		map->list[loc]->func.param_ident = malloc(sizeof(char*)*amount);
		for (int i=0;i<amount;i++)
		{
			map->list[loc]->func.param_ident[i] = malloc(11);
			strcpy(map->list[loc]->func.param_ident[i], temp_ident[i]);
		}
	}
	if (s->tok != CLOSEP)
		printf("[%ld] Expected ']' to close parameter declaration\n", s->line), exit(1);
}

void opp_parse_func(struct Scan* s)
{
	next(s);
	if (s->tok != IDENT)
		printf("[%ld] Expected Identifier after 'func'\n", s->line), exit(1);
	
	unsigned int loc = hash_str(s->lexeme);

	if (!insert_func(map, loc, s->lexeme))
		printf("[%ld] Function '%s' already defined\n", s->line, s->lexeme), exit(1);

	next(s);
	opp_parse_param(s, loc);

	next(s);
	if (s->tok != OPENB)
		printf("[%ld] Expected '{' at function declaration '%s'\n", s->line, map->list[loc]->key), exit(1);
	map->list[loc]->func.loc = s->src;
	opp_ignore(s);
}

void opp_parse_fncall(struct Scan* s, struct Table* local)
{
	unsigned int loc = hash_str(s->lexeme);

	if (map->list[loc] == NULL)
		printf("[%ld] '%s' Not a defined 'func'\n", s->line, s->lexeme), exit(1);
	if (map->list[loc]->type != FUNC)
		printf("[%ld] '%s' Not of type function\n", s->line, s->lexeme), exit(1);

	next(s);	

	if (s->tok != OPENP)
		printf("[%ld] Expected '[' when calling function '%s'\n", s->line, map->list[loc]->key), exit(1);

	if (map->list[loc]->func.exp_param == 0)
	{
		next(s);
		if (s->tok != CLOSEP)
			printf("[%ld] Expected ']' when calling function '%s'\n", s->line, map->list[loc]->key), exit(1);
	}
	else 
	{
		int expected = 0;

		map->list[loc]->func.param_val = (struct Opp_Value*)
		malloc(sizeof(struct Opp_Value)*map->list[loc]->func.exp_param);

		next(s);
		if (s->tok == CLOSEP)
			printf("[%ld] Expected %d arguments for function '%s'\n", s->line, map->list[loc]->func.exp_param, map->list[loc]->key), exit(1);

		while (s->tok != CLOSEP)
		{
			if (s->tok == IDENT || s->tok == NUM)
			{
				map->list[loc]->func.param_val[expected] = opp_parse_type(s, local);
				expected++;
			}
			if (s->tok == CLOSEP && expected == map->list[loc]->func.exp_param)
				break;
			else
			{
				if (s->tok != COMMA)
					printf("[%ld] Expected ',' in function call '%s'\n", s->line, map->list[loc]->key), exit(1);
			}
			next(s);
		}
		if (expected != map->list[loc]->func.exp_param)
			printf("[%ld] Expected %d arguments for function '%s'\n", s->line, map->list[loc]->func.exp_param, map->list[loc]->key), exit(1);
	}

	next(s);
	expect_semi(s);
	char* temp = s->src; // TODO: FIX LINE NUMBERS IN CALL
	long temp_line = s->line;

	s->src = map->list[loc]->func.loc;
	s->block++; // TEMP solution
	opp_parser(s,0);
	s->src = temp;
	s->line = temp_line;
}

void opp_parse_assign(struct Scan* s, struct Table* local)
{
	unsigned int loc = hash_str(s->lexeme);

	if (map->list[loc] == NULL && local->list[loc] == NULL)
		printf("[%ld] Undeclared identifier '%s'\n", s->line, s->lexeme), exit(1);
	next(s);

	switch (s->tok)
	{
		case EQ:
			next(s);
			struct Opp_Value val = opp_parse_type(s, local);

			if (s->block > 0 && local->list[loc] != NULL)
			{
				if (val.val_type == T_INVALID)
					printf("[%ld] Invalid assigment to '%s'\n", s->line, local->list[loc]->key), exit(1);
				else if (val.val_type == T_INTEGER && local->list[loc]->type == INT)
					local->list[loc]->v1 = val.ival;
				else if (val.val_type == T_STRING && local->list[loc]->type == STRING)
					strcpy(local->list[loc]->v3, val.strval);
				else
					printf("[%ld] Invalid variable type switch '%s'\n", s->line, local->list[loc]->key), exit(1);
			}
			else 
			{
				if (val.val_type == T_INVALID)
					printf("[%ld] Invalid assigment to '%s'\n", s->line, map->list[loc]->key), exit(1);
				else if (val.val_type == T_INTEGER && map->list[loc]->type == INT)
					map->list[loc]->v1 = val.ival;
				else if (val.val_type == T_STRING && map->list[loc]->type == STRING)
					strcpy(map->list[loc]->v3, val.strval);
				else
					printf("[%ld] Invalid variable type switch '%s'\n", s->line, map->list[loc]->key), exit(1);
			}
		break;

		case TDECR:

			if (s->block > 0 && local->list[loc] != NULL)
			{
				if (local->list[loc]->type == INT)
					local->list[loc]->v1--;
				else 	
					printf("[%ld] Invalid variable type switch '%s'\n", s->line, local->list[loc]->key), exit(1);
			}	
			else {
				if (map->list[loc]->type == INT)
					map->list[loc]->v1--;
				else 	
					printf("[%ld] Invalid variable type switch '%s'\n", s->line, map->list[loc]->key), exit(1);
			}
			next(s);
		break;

		case TINCR:

			if (s->block > 0 && local->list[loc] != NULL)
			{
				if (local->list[loc]->type == INT)
					local->list[loc]->v1++;
				else 	
					printf("[%ld] Invalid variable type switch '%s'\n", s->line, local->list[loc]->key), exit(1);
			}
			else {
				if (map->list[loc]->type == INT)
					map->list[loc]->v1++;
				else 	
					printf("[%ld] Invalid variable type switch '%s'\n", s->line, map->list[loc]->key), exit(1);
			}
			next(s);
		break;
	}
	
	expect_semi(s);
}

void opp_parse_ifstmt(struct Scan* s, struct Table* local)
{
	int operator = 0;

	next(s);
	struct Opp_Value left = opp_parse_type(s, local);
	operator = s->tok;
	next(s);
	struct Opp_Value right = opp_parse_type(s, local);

	if (left.val_type != right.val_type)
		printf("[%ld] Cannot process 'if' on two different types\n", s->line), exit(1);

	switch (operator)
	{
		case EQEQ:
			if (left.val_type == T_INTEGER)
			{
				if (left.ival != right.ival)
					opp_ignore(s);
				else {
					if (s->tok != OPENB)
						printf("[%ld] Expected '{' after 'if' statment\n", s->line), exit(1);
					s->block++;
					s->local = local;

					opp_parser(s, 1);
					s->local = NULL;
				}
			}
			else if (left.val_type == T_STRING)
			{
				if (strcmp(left.strval, right.strval))
					opp_ignore(s);
				else {
					if (s->tok != OPENB)
						printf("[%ld] Expected '{' after 'if' statment\n", s->line), exit(1);
					s->block++;
					s->local = local;

					opp_parser(s, 1);
				}
			}
		break;

		case MORETHAN:
			if (left.val_type == T_INTEGER)
			{
				if (left.ival < right.ival)
					opp_ignore(s);
				else {
					if (s->tok != OPENB)
						printf("[%ld] Expected '{' after 'if' statment\n", s->line), exit(1);
					s->block++;
					s->local = local;

					opp_parser(s, 1);
				}
			}
			else 
				printf("[%ld] Invalid operation '>' on strings\n", s->line), exit(1);

		break;

		case LESSTHAN:

			if (left.val_type == T_INTEGER)
			{
				if (left.ival > right.ival)
					opp_ignore(s);
				else {
					if (s->tok != OPENB)
						printf("[%ld] Expected '{' after 'if' statment\n", s->line), exit(1);
					s->block++;
					s->local = local;

					opp_parser(s, 1);
				}
			}
			else 
				printf("[%ld] Invalid operation '<' on strings\n", s->line), exit(1);

		break;
	}
}

void opp_analize_ident(struct Scan* s, struct Table* local)
{
	opp_parse_assign(s, local);

	// else if (block == 1)
	// {
	// 	int i=0;
	// 	int found = 0;
	// 	unsigned int loc = hash_str(s->lexeme);
	// 	if (map->list[loc]->func.exp_param == 0)
	// 		printf("[%ld] Unexpected variable '%s' in function '%s'\n", s->line, s->lexeme, map->list[loc]->key), exit(1);

	// 	for (i=0;i<map->list[loc]->func.exp_param;i++)
	// 	{
	// 		if (!strcmp(s->lexeme, map->list[loc]->func.param_ident[i])) {
	// 			found = 1;
	// 			break;
	// 		}
	// 	}
	// 	if (found != 1)
	// 		printf("[%ld] Variable '%s' in function '%s' is not a declared variable\n", s->line, s->lexeme, map->list[loc]->key), exit(1);

	// }

}

void opp_std_print(struct Scan* s, struct Table* local)
{
	next(s);
	struct Opp_Value val = opp_parse_type(s, local);

	switch (val.val_type)
	{
		case T_INVALID:
			printf("[%ld] Unknown parameter in '%s'\n", s->line, __FUNCTION__); 
			exit(1);
		break;

		case T_INTEGER:
			printf("%d\n", val.ival);
		break;

		case T_STRING:
			printf("%s\n", val.strval);
		break;
	}
	expect_semi(s);
}

void opp_std_input(struct Scan* s, struct Table* local)
{
	// CHANGE FOR HIGHER LIMIT
	char imput_limit[100];
	imput_limit[0] = '\0';
	next(s);

	if (s->tok != IDENT)
		printf("[%ld] Expected variable as argument in '%s'\n", s->line, __FUNCTION__), exit(1);
	unsigned int loc = hash_str(s->lexeme);
	if (map->list[loc] == NULL && local->list[loc] == NULL)
		printf("[%ld] Undeclared variable '%s' as argument in '%s'\n", s->line, s->lexeme, __FUNCTION__), exit(1);
	fgets(imput_limit, 100, stdin);

	// ADD CONVERT TO NUMBER LATER
	if (s->block > 0 && local->list[loc] != NULL)
	{
		if (local->list[loc]->type == STRING)
		{
			local->list[loc]->v3 = malloc(sizeof(char)*strlen(imput_limit));
			strcpy(local->list[loc]->v3, imput_limit);
		}
		else 
			printf("[%ld] Invalid attempt to change variables '%s' type '%s'\n", s->line, s->lexeme, __FUNCTION__), exit(1);
	}
	else 
	{
		if (map->list[loc]->type == STRING)
		{
			//if (map->list[loc]->v3 == NULL)
			map->list[loc]->v3 = malloc(sizeof(char)*strlen(imput_limit));
			strcpy( map->list[loc]->v3, imput_limit);
		}
		else 
			printf("[%ld] Invalid attempt to change variables '%s' type '%s'\n", s->line, s->lexeme, __FUNCTION__), exit(1);
	}
	next(s);
	expect_semi(s);
}

void opp_init_stdlib()
{
	// Init STD Library
	if (!insert_Cfunc(map, "print", opp_std_print))
		printf("INTERNAL ERROR [%s]\n", __FUNCTION__), exit(1);
	if (!insert_Cfunc(map, "input", opp_std_input))
		printf("INTERNAL ERROR [%s]\n", __FUNCTION__), exit(1);

}

void opp_init_parser(struct Scan* s)
{
	opp_init_stdlib();
	opp_parser(s, 0);
}

void opp_parser(struct Scan* s, int use_scan)
{
	struct Table* local = createMap(SMALLER_HASH);

	for (;;)
	{
		next(s);

		switch (s->tok)
		{
			case FEND:
				return;
			break;

			case CLOSEB:
				if (s->block == 0)
					printf("[%ld] Unexpected '}'\n", s->line), exit(1);
				else 
				{
					s->block--;
					return;
				}
			break;

			case TVAR:
				if (use_scan == 1)
					opp_parse_var(s, s->local);
				else opp_parse_var(s, local);
			break;

			case TIF:
				if (use_scan == 1)
					opp_parse_ifstmt(s, s->local);
				else opp_parse_ifstmt(s, local);
			break;

			case TFUNC:
				opp_parse_func(s); // decide on making func local??
			break;

			default:
			{
				int ident_type = 0;

				if (s->block > 0)
				{
					if (use_scan == 1)
						ident_type = check_type(s->local, s->lexeme);
					else ident_type = check_type(local, s->lexeme);

					if (ident_type == ERROR)
						ident_type = check_type(map, s->lexeme);
				}
				else ident_type = check_type(map, s->lexeme);

				if (ident_type == ERROR)
				{
					if (strlen(s->lexeme) == 0)
						printf("[%ld] Unexpected Token:%d\n", s->line, s->tok), exit(s->tok);
					else printf("[%ld] Unknown Identifier '%s'\n", s->line, s->lexeme), exit(1);
				}
				else if (ident_type == FUNC)
				{
					opp_parse_fncall(s, local);
				}
				else if (ident_type == CFUNC)
				{
					unsigned int loc = hash_str(s->lexeme);

					if (use_scan == 1)
						(*map->list[loc]->func.cfn)(s, s->local);
					else (*map->list[loc]->func.cfn)(s, local);
				}
				else
				{
					if (use_scan == 1)
						opp_analize_ident(s, s->local);
					else opp_analize_ident(s, local);
				}
			}
		}
	}
}