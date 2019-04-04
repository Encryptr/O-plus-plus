
int var_count = 0;
int var_num_count = 0;

typedef enum
{
  VAR,
  EQ,
  TYPE,
} Tok;

typedef struct
{
	int val[10];
	char var_name[10][10];

} Variable;
