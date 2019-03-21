#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LENGTH 1000

typedef enum 
{
  FIND_CLASS,
  COPY_CONT,
  FIND_ALL,
} States;

char tokens[100][100] = {};
int idx = 0;

int main(int argc, char *argv[])
{
  States state = FIND_CLASS;
  FILE *file;

  file = fopen(argv[1], "rt");
  if (file == NULL)
  {
    printf("NO FILE %s\n", argv[1]);
    return -1;
  }

  char fline[255];
  //char tok[50];
  
  //Condition
  const char *str_delim = " \n";

  while (fgets(fline, MAX_LENGTH, file) != NULL)
  {
  	
    char *sword = strtok(fline, str_delim);

    while (sword != NULL)
    {
      printf("--->[%s] state=%d\n", sword, state);

      switch(state)
      {

        case FIND_CLASS:

          if (strcmp(sword, "class") == 0)
          {
            state = COPY_CONT;
          }
          else 
          {
            printf("NO CLASS continuing...\n");
            state = FIND_ALL;
          }
        break;

        case COPY_CONT:
          if (strcmp(sword, "end") == 0)
          {
            // PRINT ALL INSIDE CLASS
            for (int i=0; i<idx; ++i)
            { 
              printf("%s\n", tokens[i]); 
            }
            exit(0);
          }
          strcpy(tokens[idx], sword);
          idx++;
        break;

        case FIND_ALL:
          if (strcmp(sword, "print") == 0)
          {
            // ### FIX NOT REGOGNITION AND ADD "" ###
          } else
          {
            printf("SYNTAX ERROR %s", sword);
          }
        break;
      }

      sword = strtok(NULL, str_delim);
    }
  }

}
