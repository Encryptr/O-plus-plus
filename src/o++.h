
typedef enum
{
  VAR,
  EQ,
  INTEGER,
  FLOAT,

} Tok;

// Make *struct for all varaibles
typedef struct
{
	int val;
	char var_name;

} Variable;
