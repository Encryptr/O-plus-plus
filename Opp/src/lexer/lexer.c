#include "lexer.h"

static inline bool ignore(char i) {
	return (i == ' ' || i == '\t' || i=='\r');
}

static inline bool isletter(char i) {
	return ((i >= 'a' && i <= 'z') || 
		(i >= 'A' && i <= 'Z') || (i == '_'));
}

static inline bool isnum(char i) {
	return ((i >= '0' && i <= '9') || i == '.');
}

const char* opp_keys[] = {
	"const", "if", "while",
	"true", "false", "func", 
	"else", "var", "import"
};

void append(char *og, char c)
{
	int len = strlen(og);
	og[len+1] = og[len];
	og[len] = c;
}

void opp_init_lex(struct Opp_Scan *s, char* source)
{
	s->src = source;
	s->line++;
}

void opp_preprocessor(struct Opp_Scan *s)
{
	char* pre = s->src;

	while (*pre)
	{
		if (*pre == '/') {
			pre++;
			if (*pre == '/') {
				while (*pre != '\n') {
					*pre = ' ';
					pre++;
				}
			}
			else pre--;
		}
		pre++;
	}
}

static enum Opp_Token opp_singlechar(struct Opp_Scan *s)
{
	switch (*s->src)
	{ 
		case '"': s->src++; lex_str(s); return STR;
		case '(': return TOPENP;
		case ')': return TCLOSEP;
		case ',': return TCOMMA; 
		case '[': return TOPENB;
		case ']': return TCLOSEB;
		case ';': return TSEMICOLON; 
		case '{': return TOPENC;
		case '}': return TCLOSEC;
		case '.': return TDOT;
		case '#': return THASH;

		case '>': {
			s->src++;
			if (*s->src == '=')
				return TGE;
			s->src--;
			return TGT;
		}

		case '<': {
			s->src++;
			if (*s->src == '=')
				return TLE;
			s->src--;
			return TLT;
		}

		case '&': {
			s->src++;
			if (*s->src == '&')
				return TAND;
			s->src--;
			return TADDR;
		}

		case '|': {
			s->src++;
			if (*s->src == '|')
				return TOR;
			s->src--;
		}

		case '=': {
			s->src++;
			if (*s->src == '=')
				return TEQEQ;
			s->src--;
			return TEQ;
		}

		case '!': {
			s->src++;
			if (*s->src == '=')
				return TNOTEQ;
			s->src--;
			return TNOT;
		}

		case '+': {
			s->src++;
			if (*s->src == '+')
				return TINCR;
			else if (*s->src == '=')
				return TADDEQ;
			s->src--;
			return TADD;
		}

		case '-': {
			s->src++;
			if (*s->src == '-')
				return TDECR;
			else if (*s->src == '=')
				return TMINEQ;
			else if (*s->src == '>')
				return TARROW;
			s->src--; 
			return TMIN;
		}

		case '/': {
			s->src++;
			if (*s->src == '=')
				return TDIVEQ;
			s->src--;
			return TDIV;
		}

		case '*': {
			s->src++;
			if (*s->src == '=')
				return TMULEQ;
			s->src--;
			return TMUL;
		}

		case '%': {
			s->src++;
			if (*s->src == '=')
				return TMODEQ;
			s->src--;
			return TMOD;
		}

		default:
			if ((int)*s->src > 126) 
				opp_error(s, "Invalid Token Id: '%d'", (int)*s->src);
			else 
				opp_error(s, "Invalid Token '%c'", *s->src);
	}
	return INVALID;
}

static void lex_str(struct Opp_Scan* s)
{
	if (all_until('"', s) != 1)
		opp_error(s, "Missing teminating \"");
}

static bool opp_keyword(struct Opp_Scan *s)
{
	if (!strcmp(s->lexeme, opp_keys[0])) {
		s->tok = TCONST;
		return true;
	}

	else if (!strcmp(s->lexeme, opp_keys[1])) {
		s->tok = TIF;
		return true;
	}

	else if (!strcmp(s->lexeme, opp_keys[2])) {
		s->tok = TWHILE;
		return true;
	}

	else if (!strcmp(s->lexeme, opp_keys[3])) {
		s->tok = TTRUE;
		return true;
	}

	else if (!strcmp(s->lexeme, opp_keys[4])) {
		s->tok = TFALSE;
		return true;
	}

	else if (!strcmp(s->lexeme, opp_keys[5])) {
		s->tok = TFUNC;
		return true;
	}

	else if (!strcmp(s->lexeme, opp_keys[6])) {
		s->tok = TELSE;
		return true;
	}

	else if (!strcmp(s->lexeme, opp_keys[7])) {
		s->tok = TVAR;
		return true;
	}

	else if (!strcmp(s->lexeme, opp_keys[8])) {
		s->tok = TIMPORT;
		return true;
	}

	return false;
}

static void identifier(struct Opp_Scan *s)
{
	int len = 0;
	while (isletter(*s->src))
	{
		if (len >= MAX_LEXEME_SIZE) 
			opp_error(s, "Buffer overflow '%c%c%c...'", s->lexeme[0], s->lexeme[1], s->lexeme[2]);
		append(s->lexeme, *s->src);
		s->src++;
		len++;
	}
	s->src--;
	if (opp_keyword(s)) { 
		s->src++; 
		return;
	}
	s->tok = IDENT;
	s->src++;
	return;
}

static void lex_num(struct Opp_Scan* s)
{
	int type = 0;

	while (isnum(*s->src))
	{
		if (*s->src == '.') type = FLOAT;
		else if (*s->src == 'x' || *s->src == 'X')
			opp_error(s, "Hex not supported yet!!");
		append(s->lexeme, *s->src);
		s->src++;
	}
	if (type == 0) type = INTEGER;
	s->src--;
	s->tok = type;
	s->src++;
	return;
}

int all_until(const char end, struct Opp_Scan *s)
{
	s->lexeme[0] = '\0';

	while (*(s->src) != end)
	{
		if (*s->src == EOF) 
			return 0;
		append(s->lexeme, *s->src);
		++s->src;
	}
	return 1;
}

void opp_next(struct Opp_Scan *s)
{
	s->lexeme[0] = '\0';
	while (*s->src)
	{
		if (*s->src == '\n') s->line++;
		else if (ignore(*s->src)) {}
		else if (isletter(*s->src))
			{identifier(s); return;}
		else if (isnum(*s->src))
			{lex_num(s); return;}
		else {	
			s->tok = opp_singlechar(s); 
			s->src++;
			return;
		}
		s->src++;
	}
	s->tok = FEND;
}

void dump_tokens(struct Opp_Scan *s)
{
	while (s->tok != FEND)
	{
		opp_next(s);
		if (s->tok == FEND) return;
		else if (strlen(s->lexeme) == 0)
			printf("[%u] Token #%d\n", s->line, s->tok);
		else
			printf("[%u] Token %s #%d\n", s->line, s->lexeme, s->tok);
	} 
}