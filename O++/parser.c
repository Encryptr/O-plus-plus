#include "parser.h"

#define CLEAR_STR(a) (a[0] = '\0')

struct Obj* make(int type, int num)
{
	struct Obj* obj = (struct Obj*)malloc(sizeof(struct Obj));
	obj->type = type;
	obj->num = num;

	return obj;
}

struct Obj* binary_cond(struct Scan *d, int type)
{
	struct Obj* obj = make(type, 0);
	obj->cdr = analize(d);
	return obj;
}

struct Obj* con(struct Scan *d)
{
	struct Obj* con;
	con = make(CON, 0);
	con->car = analize(d);
	con->cdr = analize(d);

	return con;
}

struct Obj* number(struct Scan *d)
{
	int number = 0;
	struct Obj* obj;
	number = atoi(d->lexeme);
	obj = make(NUM, number);
	obj->cdr = analize(d);

	return obj;
}

struct Obj* string(struct Scan *d)
{
	struct Obj* obj;
	
	if (all_until(39, d) != 1)
		printf("[%ld] ", d->line), PRINT_ERROR(5);

	obj = make(IDENT, 0);
	strcpy(obj->string, d->lexeme);
	
	next(d);

	if (d->tok != TIK)
		printf("[%ld] ", d->line), PRINT_ERROR(5);

	obj->cdr = analize(d);

	return obj;
}

struct Obj* list_make(struct Scan *d)
{
	struct Obj* obj;
	obj = make(TLIST, 0);
	obj->cdr = analize(d);

	return obj;
}

struct Obj* import(struct Scan *d)
{
	struct Obj* obj;
	obj = make(TIMPORT, 0);

	next(d);
	if (d->tok != TIK)
		PRINT_ERROR(1);

	if (all_until(39, d) != 1)
		printf("[%ld] ", d->line), PRINT_ERROR(5);

	if (d->lexeme[0] != '\0') 
		strcpy(obj->string, d->lexeme);

	next(d);

	if (d->tok != TIK)
		printf("[%ld] ", d->line), PRINT_ERROR(5);

	obj->cdr = analize(d);

	return obj;
}

struct Obj* defvar(struct Scan *d)
{
	struct Obj* obj;
	char varname[11];
	varname[0] = '\0';

	next(d);

	if (d->tok != IDENT)
		printf("[%ld]", d->line), PRINT_ERROR(6);

	strcpy(varname, d->lexeme);
	next(d);

	if (d->tok == NUM)
	{
		int number = 0;
		number = atoi(d->lexeme);
		if (!insert_int(map, varname, number))
			printf("[%ld] [%s] ", d->line, varname), PRINT_ERROR(7);
	}
	else if (d->tok == TIK)
	{
		if (all_until(39, d) != 1)
			printf("[%ld] ", d->line), PRINT_ERROR(5);
		insert_str(map, varname, d->lexeme);

		next(d);

		if (d->tok != TIK)
			printf("[%ld] ", d->line), PRINT_ERROR(5);
	}
	else {
		printf("[%ld] Invalid type initializing to var\n", d->line);
		exit(1);
	}

	obj = make(NUM, 0);
	obj->cdr = analize(d);

	return obj;
}

struct Obj* variable(struct Scan *d)
{
	struct Obj* obj;

	unsigned int loc = hash_str(d->lexeme);
	if (map->list[loc] == NULL)
	{
		printf("[%ld] Variable [%s] not defined\n", d->line, d->lexeme);
		exit(1);
	}
	switch (map->list[loc]->type)
	{
		case INT: 
			obj = make(NUM, map->list[loc]->v1);
		break;

		case STRING:
			obj = make(IDENT, 0);
			strcpy(obj->string, map->list[loc]->v3);
		break;
		// ADD FLOATS
	}
	obj->cdr = analize(d);

	return obj;
}

struct Obj* analize(struct Scan* d)
{
	for (;;)
	{
		next(d);
		switch (d->tok) 
		{
			case FEND: case CPER: 
			{
				struct Obj* nil = make(NIL, 0);
				return nil;
			}
			case NUM: 
			{
				struct Obj* num = number(d);
				return num;
			}
			case OPER: 
			{
				struct Obj* newcon = con(d);
				return newcon;
			}
			case TIK: 
			{
				struct Obj* str = string(d);
				return str;
			}
			case TLIST:
			{
				struct Obj* tlist = list_make(d);
				return tlist;
			}
			case TIF:
			{
				struct Obj* ifcond = binary_cond(d,TIF);
				return ifcond;
			}
			case TIMPORT:
			{
				struct Obj* newfile = import(d);
				return newfile;
			}
			case TVAR:
			{
				struct Obj* var = defvar(d);
				return var;
			}
			case IDENT:
			{
				struct Obj* obj = variable(d);
				return obj;
			}
			case PLUS: case MINUS: case TIMES: case DIVIDE: 
			case LESSTHAN: case MORETHAN: case EQ: case TPRINT:
			{
				struct Obj* sym = make(d->tok, 0);
				sym->cdr = analize(d);
				return sym;
			}
			default:
				if (strlen(d->lexeme) == 0)
					printf("[%ld] Invalid Statment: '%d'\n",d->line, d->tok), exit(d->tok);
				printf("[%ld] Invalid Statment: '%s'\n",d->line, d->lexeme);
				exit(1);
			break;

		}
	}
}

void warning_dump(struct Scan *d)
{
	if (d->paren_idx != 0)
		printf("\x1b[31m* WARNING *\x1b[0m Parentheses Unmatched!\n");
}
