#ifndef OHH
#define OHH

#define MAX 100

typedef enum 
{
		
	Token_Invalid, Tid, TOp, TNum,
	TVar, TPrint, TString, TEnd,

} Tokens;



typedef struct
{
	Tokens tok;
	char *src;
	char *comt;
	char lexeme[MAX];
	long line;
} OPP;

typedef struct 
{
	
} Parse_OPP;

const char* keys[] = {
	"print", "@"
};


#endif