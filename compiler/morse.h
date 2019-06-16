#ifndef MORSE
#define MORSE
#include "error.c"

#define LENGTH 10
#define SMALL 100
#define MAX 10000
#define BOOL short
#define true 1
#define false 0


BOOL isC = false;
int loop;
char *content;
int pos;

typedef enum 
{
	IDENT = 1,
	OPERATOR,
	SEMICO,
	NUM,
	VAR,
	SEPARTOR,
	TIK,
	TPRINT,
	EQ,
	EQEQ
} Tok;


typedef struct 
{
	char* src;
	long line;
	char lexeme[SMALL][SMALL];
	int tokens[MAX];
} Morse;

typedef struct 
{
	int position;
	char string[SMALL];
	long len;
} Parser;

typedef struct 
{
	int val[LENGTH];
	char name[LENGTH][LENGTH];
	int amount;
} Variable;

typedef struct 
{
	FILE *fp;
	char filename[10];
} Gen_Asm;

void gen_print(Gen_Asm *ga, char* str);


#endif