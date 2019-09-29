
char *content = NULL;
int pos = 0;

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
	TIF,
	TFUNC,
	TRET,
	TWHILE,
	EQ,
	EQEQ,
	PLUS,
	MINUS,
	TIMES,
	DIVIDE,
	OBRACK,
	CBRACK,
	OPER,
	CPER,
	COMMA,
	LOCAL
} Tok;

typedef struct 
{
	char* src;
	long line;
	char lexeme[SMALL][SMALL];
	int tokens[MAX];
	int stmt;
} Morse;