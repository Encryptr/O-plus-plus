#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "o++.h"
#include "class.c"
#include "lexer.c"

#define MAX_LENGTH 1000

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
    check_main(fline);
  }

  rewind(file);

  while (fgets(fline, MAX_LENGTH, file) != NULL)
  {
    main_lex(fline, file);
  }
}
