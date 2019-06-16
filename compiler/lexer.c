#include "morse.h"

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

// BOOL isId(char c)
// {
// 	if (c == '.' || c == '-') return true;
// 	return false;
// }

BOOL isOp(char c)
{
	if (c == '>') return true;
	return false;
}

BOOL isLetter(char c)
{
	if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') return true;
	return false;
}

BOOL ignore(char c)
{
	if (c == ' ' || c == '\t') return true;
	return false;
}

BOOL isNum(char c)
{
	if (c >= '0' && c <= '9') return true;
	return false;
}

void lex_init(Morse *m, char* source)
{
	// if (source == NULL) return;
	//m->lexeme[0] = '\0';
	m->src = source;
	m->line++;
}

void preprocessor(Morse *m)
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
		if (strcmp(m->lexeme[pos], "print") == 0)
		{
			m->tokens[pos] = TPRINT;
			pos++;
			return;
		}
		m->src--;
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
			case '@':
				strcpy(m->lexeme[pos], "@");
				m->tokens[pos] = VAR;
				pos++;
				return;
			break;

			case ';':
				strcpy(m->lexeme[pos], ";");
				m->tokens[pos] = SEMICO;
				pos++;
				return;
			break;

			case 39:
				strcpy(m->lexeme[pos], "quote");
				m->tokens[pos] = TIK;
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
				strcpy(m->lexeme[pos], "=");
				pos++;
				return;
			break;

		}
	}

	printf("ERROR TOKEN [line %d]\n", m->line);
	printf("==>%c\n", *m->src);
	exit(1);

}
