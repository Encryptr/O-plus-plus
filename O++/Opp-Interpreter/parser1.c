#include "parser.h"

void expect_semi(struct Scan* s)
{
	if (s->tok != SEMICOLON)
		printf("[%ld] Expected ';'\n", s->line), exit(1);
}

struct Opp_Value opp_parse_type(struct Scan* s)
{
	struct Opp_Value value = {0};
	
	if (s->tok == IDENT)
	{
		struct Opp_Value temp = {0};

		temp = opp_parse_varstr(s);
		if (temp.val_type != T_INTEGER)
		{
			temp.val_type = T_STRING;
			return temp;
		}
	}

	if (s->tok == NUM || s->tok == IDENT)
	{
		value = opp_expr_one(s);
		value.val_type = T_INTEGER;
	}
	else if (s->tok == TIK)
	{
		value = opp_parse_str(s);
		value.val_type = T_STRING;
	}
	else value.val_type = T_INVALID;

	return value;
}

struct Opp_Value opp_expr_one(struct Scan* s)
{
	// next(s);
	struct Opp_Value right = {0};
	struct Opp_Value left  = {0};
	int operator = 0;

	left = opp_expr_two(s);
	operator = s->tok;

	while (operator == PLUS || operator == MINUS)
	{
		next(s);
		right = opp_expr_two(s);

		if (operator == PLUS)
			left.ival += right.ival;
		else if (operator == MINUS)
			left.ival -= right.ival;
		operator = s->tok;
	}
	return left;
}

struct Opp_Value opp_expr_two(struct Scan* s)
{
	struct Opp_Value right = {0};
	struct Opp_Value left  = {0};
	int operator = 0;

	opp_parse_num(s, &left);
	next(s);
	operator = s->tok;

	while (operator == DIVIDE || operator == MULTI)
	{
		next(s);
		opp_parse_num(s, &right);

		if (operator == DIVIDE)
			left.ival /= right.ival;
		else if (operator == MULTI)
			left.ival *= right.ival;
		next(s);
		operator = s->tok;
	}
	return left;
}

void opp_parse_num(struct Scan* s, struct Opp_Value* value)
{
	// struct Opp_Value value = {0};

	if (s->tok == NUM)
		value->ival = atoi(s->lexeme);
	else if (s->tok == IDENT)
	{
		unsigned int loc = hash_str(s->lexeme);

		if (map->list[loc] == NULL)
			printf("[%ld] Undeclared identifier '%s'\n", s->line, s->lexeme), exit(1);
		if (map->list[loc]->type != INT)
			printf("[%ld] Variable '%s' must be a number\n", s->line, s->lexeme), exit(1);
		value->ival = map->list[loc]->v1;
	}
	// return value;
}

struct Opp_Value opp_parse_varstr(struct Scan* s)
{
	struct Opp_Value value = {0};
	unsigned int loc = hash_str(s->lexeme);

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

	return value;
}

struct Opp_Value opp_parse_str(struct Scan* s)
{
	struct Opp_Value str = {0};

	if (all_until('\'', s) == 0)
		{ printf("[%ld] No terminating [']\n", s->line); exit(1); }

	str.strval = malloc(sizeof(char)*(strlen(s->lexeme)+1));
	strcpy(str.strval, s->lexeme);
	next(s);
	next(s);

	return str;
}

void opp_parse_var(struct Scan* s)
{
	char varname[10];
	varname[0] = '\0';
	struct Opp_Value val = {0};

	next(s);

	if (s->tok != IDENT)
		printf("[%ld] Expected identifier after 'var'\n", s->line), exit(1);
	strcpy(varname, s->lexeme);
	unsigned int loc = hash_str(s->lexeme);
	if (map->list[loc] != NULL)
		printf("[%ld] Variable '%s' alread defined\n", s->line, s->lexeme), exit(1);
	next(s);

	if (s->tok != EQ)
		printf("[%ld] Expected '=' after 'var'\n", s->line), exit(1);
	next(s);

	val = opp_parse_type(s);

	switch (val.val_type)
	{
		case T_INVALID:
			printf("[%ld] Unknow type in var declaration\n", s->line); 
			exit(1);
		break;

		case T_INTEGER:
			insert_int(map, varname, val.ival);
		break;

		case T_STRING:
			insert_str(map, varname, val.strval);
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
		else if (s->tok == OPENB) loop++;
		else if (s->tok == CLOSEB) loop--;
		if (loop == 0) break;

		next(s);
	}
}

// ADD FUNCTION PAREMETERS
void opp_parse_param(struct Scan* s, unsigned int loc)
{
	int amount = 0;
	if (s->tok != OPENP)
		printf("[%ld] Expected '[' in function declaration\n", s->line), exit(1);
	do {
		next(s);
	} while (s->tok != CLOSEP);
}

void opp_parse_func(struct Scan* s)
{
	next(s);
	if (s->tok != IDENT)
		printf("[%ld] Expected Identifier after 'func'\n", s->line), exit(1);
	
	unsigned int loc = hash_str(s->lexeme);

	if (!insert_func(map, loc))
		printf("[%ld] Function '%s' already defined\n", s->line, s->lexeme);

	next(s);
	// === TEMP ==============
	if (s->tok != OPENP)
		printf("TEMP ERROR1 [%ld]\n", s->line), exit(1);
	next(s);
	if (s->tok != CLOSEP)
		printf("TEMP ERROR2 [%ld]\n", s->line), exit(1);
	// opp_parse_param(s, loc);
	// =======================

	next(s);
	if (s->tok != OPENB)
		printf("[%ld] Expected '{' at function declaration\n", s->line), exit(1);
	map->list[loc]->func.loc = s->src;
	opp_ignore(s);
}

void opp_parse_fncall(struct Scan* s)
{
	unsigned int loc = hash_str(s->lexeme);

	if (map->list[loc] == NULL)
		printf("[%ld] '%s' Not a defined 'func'\n", s->line, s->lexeme), exit(1);
	if (map->list[loc]->type != FUNC)
		printf("[%ld] '%s' Not of type function\n", s->line, s->lexeme), exit(1);
	// TEMP ====
	next(s);
	if (s->tok != OPENP) printf("TEMP3\n"), exit(1);
	next(s);
	if (s->tok != CLOSEP) printf("TEMP4\n"), exit(1);
	// =========

	next(s);
	expect_semi(s);
	char* temp = s->src;
	s->src = map->list[loc]->func.loc;
	opp_parser(s, 1);
	s->src = temp;
}

void opp_std_print(struct Scan* s)
{
	next(s);
	struct Opp_Value val = opp_parse_type(s);

	switch (val.val_type)
	{
		case T_INVALID:
			printf("[%ld] Unknow parameter in '%s'\n", s->line, __FUNCTION__); 
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

void opp_init_stdlib()
{
	// Init STD Library
	if (!insert_Cfunc(map, "print", opp_std_print))
		printf("INTERNAL ERROR [%s]\n", __FUNCTION__), exit(1);
}

void opp_init_parser(struct Scan* s)
{
	opp_init_stdlib();
	opp_parser(s, 0);
}

void opp_parser(struct Scan* s, int block)
{
	for (;;)
	{
		next(s);

		switch (s->tok)
		{
			case FEND:
				return;
			break;

			case CLOSEB:
			 if (block != 1)
			 	printf("[%ld] Unexpected '}'\n", s->line), exit(1);
			 return;
			break;

			case TVAR:
				opp_parse_var(s);
			break;

			case TCONST:
			
			break;

			case TFUNC:
				opp_parse_func(s);
			break;

			default:
			{
				int ident_type = check_type(map, s->lexeme);

				if (ident_type == ERROR)
				{
					if (strlen(s->lexeme) == 0)
						printf("[%ld] Unexpected TK:%d\n", s->line, s->tok), exit(s->tok);
					else printf("[%ld] Unknow Identifier '%s'\n", s->line, s->lexeme), exit(1);
				}
				else if (ident_type == FUNC)
					opp_parse_fncall(s);

				else if (ident_type == CFUNC)
				{
					unsigned int loc = hash_str(s->lexeme);
					(*map->list[loc]->func.cfn)(s);
				}
			}
		}
	}
}