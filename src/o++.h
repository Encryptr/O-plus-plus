
typedef enum
{
  VAR,
  EQ,
  TYPE,
  INTEGER,
  FLOAT,

} Tok;

// Make *struct for all varaibles
typedef struct
{
	int val;
	char var_name[10];

} Variable;
