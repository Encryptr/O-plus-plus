#include "lexer.h"

static inline bool ignore(char i) {
	return (i == ' ' || i == '\t' || i=='\r');
}

static inline bool isletter(char i) {
	return ((i >= 'a' && i <= 'z') || (i >= 'A' && i <= 'Z'));
}

static inline bool isnum(char i) {
	return ((i >= '0' && i <= '9') || i == '.');
}

void append_string(char *original, char *add)
{
   while(*original)
      original++;
     
   while(*add) {
		*original = *add;
		add++;
		original++;
   }
   *original = '\0';
}

void append(char *og, char c)
{
	int len = strlen(og);
	og[len + 1] = og[len];
	og[len] = c;
}

void init_lex(struct Scan *s, char* source) 
{
	s->src = source;
	s->line++;
}

void preprocessor(struct Scan *s)
{
	char* pre = s->src;
	char lexeme[10];

	while (*pre)
	{
		if (*pre == '#')
		{
			while (*pre != '\n') {
				*pre = ' ';
				pre++;
			}
		}
		pre++;
	}
}

bool keyword(struct Scan *s)
{
	if (!strcmp(s->lexeme, "print"))
		{s->tok = TPRINT; return 1;}
	else if (!strcmp(s->lexeme, "def"))
		{s->tok = TDEFINE; return 1;}
	else if (!strcmp(s->lexeme, "list"))
		{s->tok = TLIST; return 1;}
	else if (!strcmp(s->lexeme, "if"))
		{s->tok = TIF; return 1;}
	else if (!strcmp(s->lexeme, "import"))
		{s->tok = TIMPORT; return 1;}
	else if (!strcmp(s->lexeme, "var"))
		{s->tok = TVAR; return 1;}
	else if (!strcmp(s->lexeme, "set"))
		{s->tok = TSET; return 1;}
	return 0;
}

enum Token singleChar(struct Scan *s)
{
	switch (*s->src)
	{
		case '\'': return TIK; break;

		case '+': 
			s->src++;
			if (*s->src == '+')
				return TINCR;
			s->src--;
			return PLUS; 
		break;

		case '-': 
			s->src++;
			if (isnum(*s->src))
			{
				lex_num(s, 1);
				s->src--;
				return NUM;
			}
			else if (*s->src == '-')
				return TDECR;
			s->src--;
			return MINUS; 
		break;

		case '*': return TIMES; break;

		case '/': return DIVIDE; break;

		case '=': return EQ; break;

		case '(':
			++s->paren_idx; 
			return OPER; 
		break;

		case ')':
			--s->paren_idx; 
			return CPER; 
		break;

		case '>': return MORETHAN; break;

		case '<': return LESSTHAN; break;

		default:
			printf("[%ld] Invalid Token\n", s->line);
			printf("==>%c\n", *s->src); exit(1);
		break;
	}
}

void identifier(struct Scan *s)
{
	while (isletter(*s->src))
	{
		append(s->lexeme, *s->src);
		s->src++;
	}
	s->src--;
	if (keyword(s))
	{s->src++; return;}
	s->tok = IDENT;
	s->src++;
	return;
}

void lex_num(struct Scan* s, bool neg)
{
	if (neg == 1)
	{
		append(s->lexeme, '-');
	}
	while (isnum(*s->src))
	{
		append(s->lexeme, *s->src);
		s->src++;
	}
	s->src--;
	s->tok = NUM;
	s->src++;
	return;
}

int all_until(const char end, struct Scan *s)
{
	s->lexeme[0] = '\0';

	// TODO: ADD ERROR FOR OVER FLOWING LEXEME
	// TODO: FIX BUG WITH #

	while (*(s->src) != end)
	{
		if (*s->src == EOF) 
		{
			return 0;
		}
		append(s->lexeme, *s->src);
		++s->src;
	}
	return 1;
}

void next(struct Scan *s)
{
	s->lexeme[0] = '\0';
	while (*s->src)
	{
		if (*s->src == '\n') s->line++;
		else if (ignore(*s->src)) {}
		else if (isletter(*s->src))
			{identifier(s); return;}
		else if (isnum(*s->src))
			{lex_num(s, 0); return;}
		else
		{
			s->tok = singleChar(s);
			s->src++;
			return;
		}
		s->src++;
	}
	s->tok = FEND;
}
