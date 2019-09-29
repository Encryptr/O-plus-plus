#include "lexer.h"

void append_string(char *original, char *add)
{
   while(*original)
      original++;
     
   while(*add)
   {
		*original = *add;
		add++;
		original++;
   }
   *original = '\0';
}

void append(char *cArr, char c)
{
	int len = strlen(cArr);

	cArr[len + 1] = cArr[len];

	cArr[len] = c;
}

static inline bool isLetter(char c)
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return true;
	return false;
}

static inline bool ignore(char c)
{
	if (c == ' ' || c == '\t') return true;
	return false;
}

static inline bool isNum(char c)
{
	if (c >= '0' && c <= '9') return true;
	return false;
}

void lex_init(Morse *m, char* source)
{ 
	m->src = source;
	m->line++;
}

int preprocessor(Morse *m)
{
	char* pre = m->src;

	while (*pre)
	{
		if (*pre == '#')
		{
			while (*pre != '\n'){
				if (strlen(pre) == *pre) break;
				*pre = ' ';
				pre++;
			}
		}
		pre++;
	}
	return 0;
}

bool keyword(Morse *m)
{
	if (!strcmp(m->lexeme[pos], "print"))
	{
		m->tokens[pos] = TPRINT;
		goto end;
	}
	else if (!strcmp(m->lexeme[pos], "let"))
	{
		m->tokens[pos] = VAR;
		goto end;
	}
	else if (!strcmp(m->lexeme[pos], "if"))
	{
		m->tokens[pos] = TIF;
		goto end;
	}
	else if (!strcmp(m->lexeme[pos], "while"))
	{
		m->tokens[pos] = TWHILE;
		goto end;
	}
	else if (!strcmp(m->lexeme[pos], "fn"))
	{
		m->tokens[pos] = TFUNC;
		goto end;
	}
	else if (!strcmp(m->lexeme[pos], "ret"))
	{
		m->tokens[pos] = TRET;
		goto end;
	}
	else return 0;

	end:
		pos++;
		return 1;
}

void interpret(Morse *m)
{
	if (*m->src == '\n')
	{
		m->line++;
		return;
	}
	else if (ignore(*m->src) == true) return;
	else if (isLetter(*m->src) == true)
	{
		while (isLetter(*m->src) == true)
		{
			append(m->lexeme[pos], *m->src);
			m->src++;
		}
		m->src--;
		if (keyword(m))
		{
			return;
		}
		m->tokens[pos] = IDENT;
		pos++;
		return;
	}
	else if (isNum(*m->src) == true)
	{
		while (isNum(*m->src) == true)
		{
			append(m->lexeme[pos], *m->src);
			m->src++;
		}
		m->src--;
		m->tokens[pos] = NUM;
		pos++;
		return;
	}
	else 
	{
		switch (*m->src)
		{
			case ';':
				strcpy(m->lexeme[pos], ";");
				m->tokens[pos] = SEMICO;
				pos++;
				return;
			break;

			case 39:
				m->tokens[pos] = TIK;
				pos++;
				return;
			break;

			case '+':
				m->tokens[pos] = PLUS;
				pos++;
				return;
			break;

			case '-':
				m->tokens[pos] = MINUS;
				pos++;
				return;
			break;

			case '*':
				m->tokens[pos] = TIMES;
				pos++;
				return;
			break;

			case '/':
				m->tokens[pos] = DIVIDE;
				pos++;
				return;
			break;

			case '=':
				m->src++;
				if (*m->src == '=') 
				{
					m->tokens[pos] = EQEQ;
					pos++;
					return;
				}
				m->src--;
				m->tokens[pos] = EQ;
				pos++;
				return;
			break;

			case '(':
				m->tokens[pos] = OPER;
				pos++;
				return;
			break;

			case ')':
				m->tokens[pos] = CPER;
				pos++;
				return;
			break;

			case ',':
				m->tokens[pos] = COMMA;
				strcpy(m->lexeme[pos], ",");
				pos++;
				return;
			break;

			case '[':
				m->tokens[pos] = OBRACK;
				pos++;
				return;
			break;

			case ']':
				m->tokens[pos] = CBRACK;
				pos++;
				return;
			break;

			case '@':
				m->tokens[pos] = LOCAL;
				pos++;
			break;

			default:
				printf("ERROR TOKEN [line %d]\n", m->line);
				printf("==>%c\n", *m->src);
				exit(1);
			break;


		}
		}
}
