#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LENGTH 1000

typedef enum 
{
  FIND_CLASS,
  COPY_CONT,
  FIND_ALL,
  COPY_STRING,
  PRINT_TOK,
  
} States;

char tokens[100][100] = {};
char class_tokens[100][100] = {};
int idx = 0;
int i;

int main(int argc, char *argv[])
{
  States state = FIND_CLASS;
  FILE *file;

  file = fopen(argv[1], "rt");
  if (file == NULL)
  {
    printf("NO FILE NAMED %s\n", argv[1]);
    return -1;
  }

  char fline[255];
  
  //Condition
  const char *str_delim_def = " \n";
  const char *str_delim = str_delim_def;

  while (fgets(fline, MAX_LENGTH, file) != NULL)
  {
  	
    char *sword = strtok(fline, str_delim);

    while (sword != NULL)
    {
      //printf("[%s] state=%d\n", sword, state);

      switch(state)
      {

        case FIND_CLASS:

          if (strcmp(sword, "class") == 0)
          {
            state = COPY_CONT;
          }
          else if (strcmp(sword, "noclass") == 0)
          {
            //printf("NO CLASS continuing...\n");
            state = FIND_ALL;
          }
        break;

        case COPY_CONT:
          if (strcmp(sword, "end") == 0)
          {
            // PRINT ALL INSIDE CLASS
            for (i=0; i<idx; ++i)
            { 
              printf("%s \n", class_tokens[i]); 
            }
            
            state = FIND_ALL;            
          }
          strcpy(class_tokens[idx], sword);
          idx++;
        break;

        case FIND_ALL:
          if (strcmp(sword, "print") == 0)
          {
            str_delim = "\n";
            state = PRINT_TOK;
          } else
          {
            printf("SYNTAX ERROR %s\n", sword);
          }
        break;

        case PRINT_TOK:
        if (sscanf(sword, " (' %[^'\n] ') ", tokens) == 1)
        {
          printf("%s \n", tokens);  
          str_delim = str_delim_def;
          state = FIND_ALL;
        }
        else 
        {
          printf("SYNTAX PRINT ERROR %s \n", sword);
          exit(1);
        }
        break;
      }

      sword = strtok(NULL, str_delim);
    }
  }

}
