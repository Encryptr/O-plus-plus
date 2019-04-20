
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "o++.h"
#include "parser.c"
//#include "lex.c"

#define MAX_LENGTH 1000

// TODO: CLEAN UP LATER!!
// void main_lex(char intake[MAX_LENGTH]);
// void lex_class(char toks[100][100]);
// void print_var(const char *varname);
// int if_statment(const char *comp);

int main(int argc, char *argv[])
{
  FILE *file;

  file = fopen(argv[1], "rt");
  if (file == NULL)
  {
    printf("NO FILE NAMED %s\n", argv[1]);
    return -1;
  }
  while (fgets(fline, MAX_LENGTH, file) != NULL)
  {
    main_lex(fline);
  }
}
