#ifndef OH
#define OH
#include "o++.c"
#include "errors.h"


int has_class = 0;
int var_count = 0;
int a;
// int var_num_count = 0;

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

Tok t = VAR;
Variable vv;
Variable *vptr = &vv;


// Move to main if you want to
void print_var(const char *varname)
{
  // printf("Vars in class %d\n", var_count);
  // printf("Vars %d\n", vars);
  for (a=0;a<var_count;a++)
  {
    if (strcmp(varname, vptr->var_name[a]) == 0)
    {
      printf("%d\n", vptr->val[a]);
      return;
    }

  }
    ERROR_FOUND(6);
    printf("->%s\n", varname);

}


#endif
