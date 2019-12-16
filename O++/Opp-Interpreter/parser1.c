#include "parser.h"

void expect_semi(struct Scan* s)
{
	if (s->tok != SEMICOLON)
		printf("[%ld] Expected ';'\n", s->line), exit(1);
}

struct Opp_Value opp_parse_type(struct Scan* s)
{
	struct Opp_Value value = {0};

	if (s->tok == NUM)
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
	if (s->tok != NUM)
		printf("[%ld] Expected number not '%s'\n", s->line, s->lexeme),exit(1);

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

	left = opp_parse_num(s);
	next(s);
	operator = s->tok;

	while (operator == DIVIDE || operator == MULTI)
	{
		next(s);
		right = opp_parse_num(s);

		if (operator == DIVIDE)
			left.ival /= right.ival;
		else if (operator == MULTI)
			left.ival *= right.ival;
		next(s);
		operator = s->tok;
	}
	return left;
}

struct Opp_Value opp_parse_num(struct Scan* s)
{
	struct Opp_Value value  = {0};

	if (s->tok == NUM)
		value.ival = atoi(s->lexeme);
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
	opp_parser(s);
}

void opp_parser(struct Scan* s)
{
	for (;;)
	{
		next(s);

		switch (s->tok)
		{
			case FEND:
				return;
			break;

			default:
			{
				int ident_type = check_type(map, s->lexeme);

				if (ident_type == ERROR)
					printf("[%ld] Unknow Identifier '%s'\n", s->line, s->lexeme), exit(1);
				else if (ident_type == FUNC)
				{
					
				}
				else if (ident_type == CFUNC)
				{
					unsigned int loc = hash_str(s->lexeme);
					(*map->list[loc]->func.cfn)(s);
				}
			}
		}
	}
}