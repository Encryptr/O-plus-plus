#include <stdio.h>
#include <string.h>

#define MAX_LENGTH 1000

typedef enum 
{
  IDLE,
  STARTING,
  STARTED,
  ENDED,
} States;

char tokens[100][100] = {};
int idx = 0;

int main(int argc, char *argv[])
{
  States state = IDLE;
  FILE *file;

  file = fopen(argv[1], "rt");
  if (file == NULL)
  {
    printf("NO FILE %s\n", argv[1]);
    return -1;
  }

  char fline[255];
  char tok[50];
  
  //Condition
  const char *str_delim = " \n";

  while (fgets(fline, MAX_LENGTH, file) != NULL)
  {
  	
    char *sword = strtok(fline, str_delim);

    while (sword != NULL)
    {

      if (strcmp(sword, "class") == 0)
      {
        state = STARTING;
      }
      else if (strcmp(sword, "end") == 0)
      {
        state = ENDED;
      }

      switch(state)
      {
        case IDLE:
        break;

        case STARTING:
          state = STARTED;
        break;

        case STARTED:
          strcpy(tokens[idx], sword);
          idx++;
        break;

        case ENDED:
          for (int i=0; i<idx; ++i) printf("%s\n", tokens[i]);
        break;
      }

      sword = strtok(NULL, str_delim);
    }
  }

}
