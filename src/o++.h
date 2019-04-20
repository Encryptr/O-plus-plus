#ifndef OH
#define OH

// TODO: ?? WHY INCLUDE O++.C
//#include "o++.c"
#include "errors.h"

// TODO: MOVE VARIABLES INTO A BIG STRUCT
int has_class = 0;
int var_count = 0;
int state_if_count = 0;
int a;

//-----------------------------------------
// STRUCTS + ENUMS
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

typedef enum
{
  IGNORE,
  FIND_CLASS,
  COPY_CONT,
  FIND_ALL,
  COPY_STRING,
  PRINT_TOK,
  IF_STATE,
} States;

typedef enum
{
  CHECK_VAR,
  EQEQ,
  NUM_CHECK,
} IFSTATE;

typedef struct
{
  char comp_var[10][10];
  int comp_val[10];

} COMPARE_IF;

/* Add all Variables later
typedef struct
{

} Data;
*/
//-----------------------------------------

const char *str_delim_def = " \t\n";
const char *str_delim;

char fline[1000];

char print_string[100][100] = {};
char print_variable[10][10] = {};
char class_tokens[100][100] = {};
char variables[100][100] = {};

// IDX IS FOR CLASS
int idx = 0;
int i;
int vars;
int ascii_amount;
// IDK MOVE SOMEWHERE
int endif_amount;
int if_result;
int if_step;


#endif
