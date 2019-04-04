#ifndef PP_VAR
#define PP_VAR

#include "o++.c"
#include "o++.h"

void print_var(const char *varname)
{
  // printf("Vars in class %d\n", var_count);
  // printf("Vars %d\n", vars);
  for (i=0;i<var_count;i++)
  {
    if (strcmp(varname, vptr->var_name[i]) == 0)
    {
      printf("%d\n", vptr->val[i]);
      return;
    }

  }
    ERROR_FOUND(6);
    printf("->%s\n", varname);

}

#endif
