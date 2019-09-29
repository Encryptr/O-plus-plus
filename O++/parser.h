typedef struct 
{
	int position;
	char string[SMALL];
	long len;
} Parser;

typedef struct 
{
	long val[LENGTH];
	char str[LENGTH][LENGTH];
	char name[LENGTH][LENGTH];
	int amount;
} Variable;

#define FUNCTION 50
typedef struct 
{
	int st_pos[FUNCTION], ed_pos[FUNCTION];
	char fn_name[20][20];
	int fn_idx;
	int iret[FUNCTION];
	char fn_param[20][20];
} Func;

void parse_begin(Morse *m, Parser *p, Variable *pv);
void parse_call(Morse *m, Parser *p, Variable *pv, Func *f, int i, int op);
void parse_print(Morse *m, Parser *p, Variable *pv, Func *f);
void parse_expr(Morse *m, Parser *p, Variable *pv);
void parse_ident(Morse *m, Parser *p, Variable *pv, Func *f);
void parse_func(Morse *m, Parser *p, Func *f);
void parse_params(const char* str, Morse *m, Parser *p, Func *f);
void parse_return(Morse *m, Parser *p, Func *f);
void parse_local_var(Morse *m, Parser *p, Func *f);
void parse_ifstmt(Morse *m, Parser *p, Variable *pv, Func *f);
void balance(Morse *m, Parser *p);