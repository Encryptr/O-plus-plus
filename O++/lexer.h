#ifndef OPP_LEXER
#define OPP_LEXER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

enum Token {
	INVALID, IDENT, NUM, TIMPORT, TVAR,
	TFUNC, TCFUNC, TCONST, TIF, TWHILE, TIK, EQ, 
	EQEQ, PLUS, MINUS, DIVIDE, MULTI, TDECR, 
	TINCR, OPENP, CLOSEP, COMMA, OPENB, CLOSEB, 
	LESSTHAN, MORETHAN, COLON, SEMICOLON, TRET, FEND
};

struct Scan {
	char* src;
	char lexeme[99];
	enum Token tok;
	long line;
	FILE* file;	
};

static inline bool ignore(char i);
static inline bool isletter(char i);
static inline bool isnum(char i);

void append(char *og, char c);
void append_string(char *original, char *add);

void preprocessor(struct Scan *s);
bool keyword(struct Scan *s);
enum Token singleChar(struct Scan *s);
void identifier(struct Scan *s);
void lex_num(struct Scan* s, bool neg);
int all_until(const char end, struct Scan *s);

char* type_to_str(const int type);
void dump_tokens(struct Scan *s);

void init_lex(struct Scan *s, char* source);
void next(struct Scan *d);
void ignore_str(struct Scan* s);

#endif