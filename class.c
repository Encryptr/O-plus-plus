#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_LENGTH 100

int idx = 0;
char tok[MAX_LENGTH];

typedef enum
{
  IDLE,
  STARTED,
  END,

} States;

States state = IDLE;

void lex(char *tok)
{
  if (strcmp(tok, "class\n") == 0)
  {
    state = IDLE;
  }

  else if (strcmp(tok, "end\n") == 0)
  {
    state = END;
  }

  switch(state)
  {
    case IDLE:
        state = STARTED;
    break;

    case STARTED:
        
    break;

    case END:


    break;

  }

}


int main() 
{
  FILE *file;
  file = fopen("text.txt", "r");
  fgets(tok, MAX_LENGTH, file);
  lex(tok);

}
