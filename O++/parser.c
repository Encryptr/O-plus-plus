#include "parser.h"

void expect_semi(struct Scan* s)
{
	if (s->tok != SEMICOLON)
		printf("[%ld] Expected ';'\n", s->line), exit(1);
}

void opp_std_print(struct Scan* s)
{
	
}

void opp_init_stdlib()
{
	// Init STD Library
	if (!insert_Cfunc(global_ns->inside, "print", opp_std_print))
		printf("INTERNAL ERROR [%s]\n", __FUNCTION__), exit(1);
}

void opp_init_parser(struct Scan* s)
{
	opp_init_stdlib();
}

void opp_declaration(struct Scan* s)
{
	switch (s->tok)
	{
		case TVAR:
		case TFUNC:
		case TIMPORT:
		break;

		default:
			opp_statment(s);
			break;
	}
}
void opp_statment(struct Scan* s)
{
	switch (s->tok)
	{
		case TIF:
		case TWHILE:
		case OPENB:
		case TRET:
		break;

		default:
			opp_expression(s);
			break;
	}
}

void opp_expression(struct Scan* s)
{

}


