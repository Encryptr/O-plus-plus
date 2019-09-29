#include "parser.h"

static int isVar(Variable *pv, char* c)
{
	for (int i=0;i<pv->amount;i++)
	{
		if (strcmp(c, pv->name[i]) == 0) return i;
	}
	return -1;
}

static int isFunc(Func *f, char* c)
{
	int i=0;
	for (i;i<f->fn_idx;i++)
	{
		if (strcmp(c, f->fn_name[i]) == 0) return i;
	}
	return -1;
}

static bool expect(int exp, Morse *m, Parser *p)
{
	if (m->tokens[p->position] == exp) return 1;
	return 0;
}

static inline void getnext(Parser *p)
{
	p->position++;
}

static bool isSemi(Morse *m, Parser *p)
{
	if (m->tokens[p->position] == SEMICO) return true;
	return false;
}

void parse_print(Morse *m, Parser *p, Variable *pv, Func *f)
{
	getnext(p);
	if (expect(TIK,m,p))
	{
		getnext(p);
		while (m->tokens[p->position] != TIK)
		{	
			append_string(p->string, m->lexeme[p->position]);
			getnext(p);
			if (m->tokens[p->position] != TIK) append_string(p->string, " ");
		}
		if (!expect(TIK, m,p)) puts("Could not find terminating ' for print statment\n"), exit(1);
		getnext(p);
		if (m->tokens[p->position] == SEMICO)
		{
			puts(p->string);
			p->string[0] = '\0';
		} else {ERROR_FOUND(3, ";", m->lexeme[p->position-1]); exit(1);}
	}
	else if (expect(IDENT,m,p))
	{
		int var_idx = 0;
		if (isVar(pv, m->lexeme[p->position]) != -1)
		{
			var_idx = isVar(pv, m->lexeme[p->position]);
			getnext(p);
			if (m->tokens[p->position] == SEMICO)
			{
				if (strcmp(pv->str[var_idx], "\0"))
				{
					printf("%s\n", pv->str[var_idx]);
					return;
				}
				printf("%ld\n", pv->val[var_idx]);
			} else {ERROR_FOUND(3, ";", m->lexeme[p->position]); exit(1);}
		}
		else if (isFunc(f, m->lexeme[p->position]) != -1)
		{
			var_idx = isFunc(f, m->lexeme[p->position]);
			getnext(p);
			if (!expect(OPER,m,p)) printf("Expected '(' at function call [%s]\n", m->lexeme[p->position]);
			getnext(p);
			if (!expect(CPER,m,p)) printf("Expected ')' at function call [%s]\n", m->lexeme[p->position]);
			getnext(p);
			if (!expect(SEMICO,m,p)) ERROR_FOUND(3, ";", m->lexeme[p->position-1]), exit(1);
			parse_call(m,p,pv,f,var_idx,2);
			printf("%d\n", f->iret[var_idx]);
		}
	}
	else 
	{
		ERROR_FOUND(2, "Incomplete print statment", m->lexeme[p->position]); 
		exit(1);
	}
}

void parse_expr(Morse *m, Parser *p, Variable *pv)
{
	getnext(p);
	if (!expect(IDENT,m,p)) SMALL_ERR(1), exit(1);
	if (isVar(pv, m->lexeme[p->position]) != -1)
	{
		printf("Error Initializing variable '%s' => already used\n", m->lexeme[p->position]);
		exit(1);
	}
	append_string(pv->name[pv->amount], m->lexeme[p->position]);
	getnext(p);
	if (!expect(EQ,m,p)) puts("Expected '='\n"), exit(1);
	getnext(p);
	if (m->tokens[p->position] == NUM)
	{
		int res = atoi(m->lexeme[p->position]); // change to atol
		pv->val[pv->amount] = res;
		getnext(p);
		if (expect(SEMICO,m,p))
		{
			res = 0;
			pv->amount++;
		} else {ERROR_FOUND(3, ";", m->lexeme[p->position]); exit(1);}
	}
	else if (m->tokens[p->position] == MINUS)
	{
		getnext(p);
		if (!expect(NUM,m,p)) puts("Expected Number after '-'\n"), exit(1);
		int res = atoi(m->lexeme[p->position]);
		res *= -1;
		pv->val[pv->amount] = res;
		getnext(p);
		if (expect(SEMICO,m,p))
		{
			res = 0;
			pv->amount++;
		} else {ERROR_FOUND(3, ";", m->lexeme[p->position]); exit(1);}
	}
	else if (m->tokens[p->position] == TIK)
	{
		getnext(p);
		while (m->tokens[p->position] != TIK)
		{
			append_string(pv->str[pv->amount], m->lexeme[p->position]);
			getnext(p);
			if (m->tokens[p->position] != TIK) append_string(pv->str[pv->amount], " ");
		}
		getnext(p);
		if (expect(SEMICO,m,p))
		{
			pv->amount++;
		} else {ERROR_FOUND(3, ";", m->lexeme[p->position]); exit(1);}
	}
}

void parse_call(Morse *m, Parser *p, Variable *pv, Func *f, int i, int op)
{
	int og_pos = p->position;
	p->position = f->st_pos[i];


	if (op == 1)
	{
		while (p->position != f->ed_pos[i])
		{
			switch(m->tokens[p->position])
			{
				case 0: continue; break;
				case TPRINT: parse_print(m, p, pv, f); break;
				case VAR: parse_expr(m, p, pv); break;
				case IDENT: parse_ident(m,p,pv,f); break;
				case TFUNC: parse_func(m,p,f); break;
				case TRET: parse_return(m,p,f); break;
				//case LOCAL: parse_local_var(m,p,f); break;

				default:
					printf("Invaid expr in function\n"); exit(1);
				break;
			}
			p->position++;
		}
	}
	else if (op == 2)
	{
		bool found = false;
		while (p->position != f->ed_pos[i])
		{
			if (m->tokens[p->position] == TRET)
			{
				parse_return(m,p,f);
				found = true;
			}
			p->position++;
		}
		if (found == false)
			printf("Couldnt find 'ret' in function -> [%s]\n", f->fn_name[i]),exit(1);
	}
	p->position = og_pos;
}

void parse_return(Morse *m, Parser *p, Func *f)
{
	int funcNum = f->fn_idx - 1;
	getnext(p);
	if (!expect(NUM,m,p)) puts("Expected Number after 'ret'\n"), exit(1);
	f->iret[funcNum] = atoi(m->lexeme[p->position]);
	getnext(p);
	if (!expect(SEMICO,m,p)) puts("Expected ';' after 'ret'\n"), exit(1);
}

void parse_local_var(Morse *m, Parser *p, Func *f)
{
	int var_idx = 0;
	getnext(p);
	if (!expect(IDENT,m,p)) puts("Expected Identifier after '@'\n"), exit(1);
	// CHECK IF IS FUNCTION and if has those parmaters

}

void parse_ident(Morse *m, Parser *p, Variable *pv, Func *f)
{
	int var_num = 0;
	int effect = 0;
	if (isVar(pv, m->lexeme[p->position]) != -1)
	{
		var_num = isVar(pv, m->lexeme[p->position]);
		getnext(p);
		switch (m->tokens[p->position])
		{
			case PLUS: 
				getnext(p);
				if ((!expect(NUM,m,p)) && (!expect(MINUS,m,p))) puts("Expected number after '+'\n"), exit(1);
				if (expect(MINUS,m,p))
				{
					getnext(p);
					effect = atoi(m->lexeme[p->position]);
					effect *= -1;
					getnext(p);
					if (!isSemi(m,p)) puts("Expected ';' after Identifier\n"), exit(1);
					pv->val[var_num] += effect;
					return;
				}
				effect = atoi(m->lexeme[p->position]);
				getnext(p);
				if (!isSemi(m,p)) puts("Expected ';' after Identifier\n"), exit(1);
				pv->val[var_num] += effect;
			break;
			case MINUS:
				getnext(p);
				if ((!expect(NUM,m,p)) && (!expect(MINUS,m,p))) puts("Expected number after '-'\n"), exit(1);
				if (expect(MINUS,m,p))
				{
					getnext(p);
					effect = atoi(m->lexeme[p->position]);
					effect *= -1;
					getnext(p);
					if (!isSemi(m,p)) puts("Expected ';' after Identifier\n"), exit(1);
					pv->val[var_num] -= effect;
					return;
				}
				effect = atoi(m->lexeme[p->position]);
				getnext(p);
				if (!isSemi(m,p)) puts("Expected ';' after Identifier\n"), exit(1);
				pv->val[var_num] -= effect;
			break;
			case TIMES:
				getnext(p);
				if ((!expect(NUM,m,p)) && (!expect(MINUS,m,p))) puts("Expected number after '*'\n"), exit(1);
				if (expect(MINUS,m,p))
				{
					getnext(p);
					effect = atoi(m->lexeme[p->position]);
					effect *= -1;
					getnext(p);
					if (!isSemi(m,p)) puts("Expected ';' after Identifier\n"), exit(1);
					pv->val[var_num] *= effect;
					return;
				}
				effect = atoi(m->lexeme[p->position]);
				getnext(p);
				if (!isSemi(m,p)) puts("Expected ';' after Identifier\n"), exit(1);
				pv->val[var_num] *= effect;
			break;
			case DIVIDE:
				getnext(p);
				if ((!expect(NUM,m,p)) && (!expect(MINUS,m,p))) puts("Expected number after '/'\n"), exit(1);
				if (expect(MINUS,m,p))
				{
					getnext(p);
					effect = atoi(m->lexeme[p->position]);
					effect *= -1;
					getnext(p);
					if (!isSemi(m,p)) puts("Expected ';' after Identifier\n"), exit(1);
					pv->val[var_num] /= effect;
					return;
				}
				effect = atoi(m->lexeme[p->position]);
				getnext(p);
				if (!isSemi(m,p)) puts("Expected ';' after Identifier\n"), exit(1);
				pv->val[var_num] /= effect;
			break;

			case EQ:
				getnext(p);
				if ((!expect(NUM,m,p)) && (!expect(MINUS,m,p))) puts("Expected number after '='\n"), exit(1);
				if (expect(MINUS,m,p))
				{
					getnext(p);
					effect = atoi(m->lexeme[p->position]);
					effect *= -1;
					getnext(p);
					if (!isSemi(m,p)) puts("Expected ';' after Identifier\n"), exit(1);
					pv->val[var_num] = effect;
					return;
				}
				effect = atoi(m->lexeme[p->position]);
				getnext(p);
				if (!isSemi(m,p)) puts("Expected ';' after Number\n"), exit(1);
				pv->val[var_num] = effect;
			break;

		}
	}
	else if (isFunc(f, m->lexeme[p->position]) != -1)
	{
		var_num = isFunc(f, m->lexeme[p->position]);
		getnext(p);
		if (!expect(OPER,m,p)) printf("ERROR PARSING Identifier [%s]\n", m->lexeme[p->position]),exit(1);
		getnext(p);
		if (strcmp(f->fn_param[var_num], "\0"))
		{
			// Inplement Paramaters
		}
		if (!expect(CPER,m,p)) printf("Syntax ERROR PARSING Identifier [%s]\n", m->lexeme[p->position]),exit(1);
		getnext(p);
		if (!expect(SEMICO,m,p)) printf("Expected ';' after function call [%s]\n", m->lexeme[p->position]),exit(1);
		parse_call(m,p,pv,f,var_num, 1);
	}
	else
	{
		printf("Invaild Indentifier declaration [%s]\n", m->lexeme[p->position]);
		exit(1);
	}
}

void parse_params(const char* str, Morse *m, Parser *p, Func *f)
{
	bool exp = false;
	if (strlen(str) == 0) return;
	while (*str)
	{
		if (isLetter(*str))
		{
			exp = true;
			while (isLetter(*str))
			{
				append(f->fn_param[f->fn_idx], *str);
				str++;
			}
			append(f->fn_param[f->fn_idx], '|');
		}
		if (*str == ',') 
		{
			str++;
			if (*str == '\0')
				puts("Error Unexpected ','\n"), exit(1);
			str--;
			if (exp == true) exp = false;
			else puts("Error Unexpected ','\n"), exit(1);
		}
		str++;
	}
}

void parse_func(Morse *m, Parser *p, Func *f)
{
	char parm[20];
	int counter = 0;
	getnext(p);
	if (!expect(IDENT,m,p)) puts("Expected Identifier after 'fn'\n"), exit(1);
	strcpy(f->fn_name[f->fn_idx], m->lexeme[p->position]);
	getnext(p);
	if (!expect(OPER,m,p)) puts("Expected '(' for function\n"), exit(1);
	getnext(p);
	while (m->tokens[p->position] != CPER)
	{
		append_string(parm, m->lexeme[p->position]);
		p->position++;
	}
	parse_params(parm,m,p,f);
	getnext(p);
	if (!expect(OBRACK,m,p)) puts("Expected '[' at function\n"), exit(1);
	getnext(p);
	f->st_pos[f->fn_idx] = p->position;
	while (m->tokens[p->position] != CBRACK)
	{
		p->position++;
	}
	f->ed_pos[f->fn_idx] = p->position;
	f->fn_idx++;
}

void parse_ifstmt(Morse *m, Parser *p, Variable *pv, Func *f)
{
	int var_num = 0, fun_num = 0;
	bool fn = 0, vr = 0;
	int cmp_val = 0;
	getnext(p);
	if (!expect(IDENT,m,p)) puts("Expected Identifier after 'if'\n"), exit(1);
	if (isVar(pv, m->lexeme[p->position]) == -1 && isFunc(f, m->lexeme[p->position]) == -1) puts("Identifier not var or func after 'if'\n"), exit(1);
	if (isFunc(f, m->lexeme[p->position]) != -1)
	{
		fun_num = isFunc(f, m->lexeme[p->position]);
		fn = 1;
	}
	else if (isVar(pv, m->lexeme[p->position]) != -1) 
	{
		var_num = isVar(pv, m->lexeme[p->position]);
		vr = 1;
	}
	getnext(p);
	if (!expect(EQEQ,m,p)) puts("Expected '==' after Identifier in 'if'\n"), exit(1);
	getnext(p);
	if (!expect(NUM,m,p)) puts("Expected Number after '=='\n"), exit(1);
	cmp_val = atoi(m->lexeme[p->position]);
	getnext(p);
	if (!expect(OBRACK,m,p)) puts("Expected '[' to open 'if'\n"), exit(1);
	if (vr == 1) 
	{
		m->stmt++; 
		if (cmp_val == pv->val[var_num])
			{ return; } 
		else 
		{
			while (1)
			{
				p->position++;
				//if (m->tokens[p->position] == OBRACK) m->stmt++;
				if (m->tokens[p->position] == CBRACK) break;
			}
		}
	}
}

void balance(Morse *m, Parser *p)
{
	if (m->stmt > 0)
	{
		m->stmt--;
		return;
	}
	puts("Not expect ']'\n"), exit(1);
}

void parse_begin(Morse *m, Parser *p, Variable *pv)
{
	// for (int i=0;i<100;i++)
	// {
	// 	printf("Token->%s Enum->%d\n", m->lexeme[i], m->tokens[i]);
	// }
	Func func = {0};
	func.fn_idx = 1;
	pv->amount = 1;
	while (p->len++ < pos)
	{
		switch(m->tokens[p->position])
		{
			case 0: continue; break;
			case TPRINT: parse_print(m, p, pv, &func); break;
			case VAR: parse_expr(m, p, pv); break;
			case IDENT: parse_ident(m,p,pv,&func); break;
			case TFUNC: parse_func(m,p,&func); break;
			case TIF: parse_ifstmt(m,p,pv,&func); break;
			case CBRACK: balance(m,p); break;

			default:
				printf("Invaid expr -> %s\n", m->lexeme[p->position]); 
				exit(1);
			break;
		}
		p->position++;
	}
}