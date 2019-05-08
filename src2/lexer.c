#include <ctype.h>
#include "o++.h"

static char* append(char *cc, const char c)
{

	int len = strlen(cc);

	cc[len + 1] = cc[len];

	cc[len] = c;

	return cc;
}
static int isSpace(const char c)
{
	if (c == ' ' || c == '\t' || c == '\r') return 1;
	return 0;
}

static int isLetter(const char c)
{
	if (c >= '0' && c <= '9') return 1;

	return 0;
}

static int isNum(const char c)
{
	if (c >= '0' && c <= '9') return 1;
	return 0;
}


void init(char* cont, OPP *lex)
{
	lex->src = cont;
	lex->comt = cont;
}

Tokens main_lex(OPP *lex)
{
	lex->line++;
	lex->lexeme[0] = '\0';
	while(*lex->src)
	{
		if (isSpace(*lex->src) == 1){}
		else if (*lex->src == '\n') lex->line++;
		else if (isNum(*lex->src) == 1) 
		{
			append(lex->lexeme, *lex->src);
			return TNum;
		}
		else if (isLetter(*lex->src)==1)
		{
			while (isLetter(*lex->src)==1)
			{
				append(lex->lexeme, *lex->src);
			}
			if (strcmp(lex->lexeme, "print") == 0)
			{
				return TPrint;
			}
			else 
			{
				return Tid;
			}
		}
		else
		{
			// ADD switch for individual chars
		}


		lex->src++;
	}

	printf("-->%s\n", lex->lexeme);
}

