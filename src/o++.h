#ifndef OH
#define OH

#define MAX_VAL 99

#include "errors.h"

void main_lex(char intake[1000], FILE *fi);
// CHANGE TOKS VAL 
void lex_class(char toks[100][100]);
void print_var(const char *varname);
int if_statment(const char *comp);
void change_variable(const char *curr);

int has_class = 0;
int var_count = 0;
int is_string;
int state_if_count = 0;
int a;

// STRUCTS + ENUMS
typedef enum
{
  VAR,
  EQ,
  TYPE,
} Tok;

typedef struct
{
  // Change size for larger same for IF vals
	int val[MAX_VAL];
  char string[MAX_VAL][MAX_VAL];
	char var_name[MAX_VAL][MAX_VAL];

} Variable;

typedef enum
{
  IGNORE,
  FIND_CLASS,
  COPY_CONT,
  FIND_ALL,
  COPY_STRING,
  PRINT_TOK,
  IF_STATE,
  ENDIF_FIND,
  CHANGE_VAR,

} States;

typedef enum
{
  CHECK_VAR,
  EQEQ,
  NUM_CHECK,
} IFSTATE;

typedef enum
{
  IDENTIFY,
  EXECUTE,
} Order_Change;


typedef struct
{
  char comp_var[MAX_VAL][MAX_VAL];
  int comp_val[MAX_VAL];

} COMPARE_IF;

typedef struct
{
  int cont;
  char change_comp_var[MAX_VAL][MAX_VAL];
  int op_result;
  int change_by_num[MAX_VAL];
  int does_match;
  int which_match;
} Change_Var;

/* Add all Variables later
typedef struct
{

} Data;
*/

const char *str_delim_def = " \t\n";
const char *str_delim;

// Line in later changed into sword
char fline[1000];

// Chars to hold tokens to parse
char print_string[100][100] = {};
char print_variable[10][10] = {};
char class_tokens[100][100] = {};
char variables[100][100] = {};

// amount of tokens in class
int idx = 0;
int i;

// Var index to print var
int vars;
// Ascii value to print
int ascii_amount;
// Variables for nested if
int endif_counter;
int if_counter;
int if_result;
// Variable for comments
int back_to_class = 0;
// idx for variable changing
int change_count;


const char *operators[] = {"+", "-", "*", "/"};
const char *stdlib[] =
{"class", "end", "noclass", "if", "endif", "print", "<", ">"};

#endif
