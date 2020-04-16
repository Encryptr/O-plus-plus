#ifndef OLISP_LEXER
#define OLISP_LEXER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_LEXEME_SIZE 100

enum Opp_Token {
	INVALID,

	IDENT, INTEGER, FLOAT, HEX, STR,

	// || && > < >= <= 
	TOR, TAND, TGT, TLT, TGE, TLE,

	// == = ! != ' "
	TEQEQ, TEQ, TNOT, TNOTEQ, TTIK, TQUOTE,	
	
	// + - / * % -- ++
	TADD, TMIN, TDIV, TMUL, TMOD, TDECR, TINCR,

	// += -= /= *= %= 
	TADDEQ, TMINEQ, TDIVEQ, TMULEQ, TMODEQ,

	// ( ) , [ ] . -> & # ; 
	TOPENP, TCLOSEP, TCOMMA, TOPENB, TCLOSEB, 
	TOPENC, TCLOSEC, TDOT, TARROW, TADDR, THASH,
	TSEMICOLON,
	
	TCONST, TIF, TELSE, TWHILE, TTRUE, TFALSE, TFUNC,
	TVAR,

	FEND
};

struct Opp_Scan {
	char* src;
	char lexeme[MAX_LEXEME_SIZE];
	enum Opp_Token tok;
	unsigned int line;
	unsigned int colum;
	FILE* file;
};

static inline bool ignore(char i);
static inline bool isletter(char i);
static inline bool isnum(char i);

void append(char *og, char c);

// Init
void opp_init_lex(struct Opp_Scan *s, char* source);
void opp_preprocessor(struct Opp_Scan *s);

// Lex
static enum Opp_Token opp_singlechar(struct Opp_Scan *s);
static void identifier(struct Opp_Scan *s);
static bool opp_keyword(struct Opp_Scan *s);
static void lex_num(struct Opp_Scan* s);
static void lex_str(struct Opp_Scan* s);
int all_until(const char end, struct Opp_Scan *s);
void opp_next(struct Opp_Scan *d);

// Debug
void dump_tokens(struct Opp_Scan *s);

#endif