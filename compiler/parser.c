#include "morse.h"

// Parser par;
// Parser *p = &par;
// Variable va;
// Variable *pv = &va;


BOOL Cmp(char* lexeme, char* keyword)
{
	if (strcmp(lexeme, keyword) == 0) return true;
	return false;
}

// int isVar(char* c)
// {
// 	for (int i=0;i<pv->amount;i++)
// 	{
// 		if (strcmp(c, pv->name[i]) == 0) return 1;
// 	}
// 	return 0;
// }

void lnPrint(Morse *m)
{
	printf("Line %d\n", m->line);
}

static int expect(int exp, Morse *m, Parser *p)
{
	if (m->tokens[p->position] == exp) return 1;
	return 0;
}

void getnext(Parser *p)
{
	p->position++;
}

int expSemi(int a)
{
	if (a == SEMICO) return 1;
	return 0;
}

void parse_print(Morse *m, Parser *p, Gen_Asm *ga)
{
	p->position++;
	if (m->tokens[p->position] == TIK)
	{
		p->position++;
		while (m->tokens[p->position] != TIK)
		{	
			append_string(p->string, m->lexeme[p->position]);
			p->position++;
			if (m->tokens[p->position] != TIK) append_string(p->string, " ");
		}
		if (m->tokens[p->position] == TIK)
		{
			p->position++;
			if (m->tokens[p->position] == SEMICO)
			{
				if (isC == true)
				{
					gen_print(ga, p->string);
					return;
				}
				printf("%s\n", p->string);
				p->string[0] = '\0';
			} else {ERROR_FOUND(3, ";", m->lexeme[p->position-1]); lnPrint(m); exit(1);}
		}
		else {ERROR_FOUND(2, "quotes", m->lexeme[p->position]); lnPrint(m); exit(1);}
	}
	// else if (expect(VAR, m) == 1)
	// {
	// 	getnext();
	// 	expect(IDENT, m);
	// 	parse_var(m->lexeme[p->position]);
	// }
	else 
	{
		ERROR_FOUND(2, "quotes", m->lexeme[p->position]); 
		lnPrint(m); 
		exit(1);
	}
}

// Variable *parse_var(Morse *m)
// {
// 	Variable *var = malloc(sizeof(Variable));
// 	var->val[0] = 33;
// 	return var;
// } 

void parse_expr(Morse *m, Parser *p, Variable *pv)
{
	getnext(p);
	if (m->tokens[p->position] == IDENT)
	{
		append_string(pv->name[pv->amount], m->lexeme[p->position]);
		getnext(p);
		if (m->tokens[p->position] == EQ)
		{
			getnext(p);
			if (m->tokens[p->position] == NUM)
			{
				int res = atoi(m->lexeme[p->position]); 
				pv->val[pv->amount] = res;
				getnext(p);
				if (expSemi(m->tokens[p->position]) == 1)
				{
					res = 0;
					pv->amount++;
				} else {ERROR_FOUND(3, ";", m->lexeme[p->position-1]);lnPrint(m); exit(1);}
			}
		}
	} else {SMALL_ERR(1); lnPrint(m); exit(1);}
}

void parse_begin(Morse *m, Parser *p, Variable *pv, Gen_Asm *ga)
{
	// for (int i=0;i<10;i++)
	// {
	// 	printf("Token->%s Enum->%d\n", m->lexeme[i], m->tokens[i]);
	// }
	while (p->len++ < pos)
	{
		// Call function to create int main()
		if (m->tokens[p->position] == TPRINT)
		{
			parse_print(m, p, ga);
		}
		else if (m->tokens[p->position] == VAR)
		{
			parse_expr(m, p, pv);
			//parse_var(m);
		}
		else if (m->tokens[p->position] == 0) continue;
		else {printf("Invaid expr -> %s\n", m->lexeme[p->position]); exit(1);}
		p->position++;
	}
}