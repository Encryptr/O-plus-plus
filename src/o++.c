#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "o++.h"

#define MAX_LENGTH 1000

void *lex_class(char toks[100][100]);

typedef enum
{
  PRINT,
  EQ,
} Tok;

typedef enum 
{
  IGNORE,
  FIND_CLASS,
  COPY_CONT,
  FIND_ALL,
  COPY_STRING,
  PRINT_TOK,
} States;

// Conditions for Ignoring not working GLOBALLY
// char *str_delim_def = " \t\n";
// char *str_delim = str_delim_def;


// Storing Tokens
char tokens[100][100] = {};
char class_tokens[100][100] = {};
char variables[100][100] = {};
char ignore[100][100] = {};
// Tokens variables
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

  const char *str_delim_def = " \t\n";
  const char *str_delim = str_delim_def;
  char fline[255];

  //int force_next_line = 0;

  while (fgets(fline, MAX_LENGTH, file) != NULL)
  {
    
    char *sword = strtok(fline, str_delim);

    while (sword != NULL)
    {
      //printf("[%s] state=%d\n", sword, state);

      switch(state)
      {

        case IGNORE:

          if (strcmp(sword, ">") == 0)
          {
            state = FIND_ALL;  
          }
          sword = "";
        break;
        
        case FIND_CLASS:
          // ## ADD IGNORE COMMENT ## TODO!!! USE CONTINUE TO TELL IT TO GO ON!!
          // if (sscanf(sword, " < %[^'\n] > ", ignore) == 1)
          // {
          //   str_delim = str_delim_def;
          //   for (i=0;i<idx;i++)
          //   {
          //     printf("%s \n", ignore);
          //   }  
          // }
          if (strcmp(sword, "class") == 0)
          {
            state = COPY_CONT;
          }
          else if (strcmp(sword, "noclass") == 0)
          {
            //printf("NO CLASS continuing...\n");
            state = FIND_ALL;
          }
          else 
          {
            printf("CLASS DECLARATION MISSING\n");
            exit(1);
          }
        break;

        case COPY_CONT:
          if (strcmp(sword, "end") == 0)
          {
            // NOT FULLY WORKING DONT NEED FOR NOW!!!!
            //lex_class(class_tokens);
            // ADD GOTO IF NEEDED
            state = FIND_ALL;            
          }
          strcpy(class_tokens[idx], sword);
          idx++;
        break;

        case FIND_ALL:
          // ## COMMENTS WORKING ##
          if (strcmp(sword, "<") == 0)
          {
            //printf("FOUND COMMENT\n");
            state = IGNORE;
          }
          else if (strcmp(sword, "print") == 0)
          {
            str_delim = "\n";
            state = PRINT_TOK;
          }
          else if (strcmp(sword, "") == 0)
          {
            state = FIND_ALL;
          }
          else
          {
            printf("SYNTAX ERROR %s\n", sword);
            exit(1);
          }
        break;

        case PRINT_TOK:
        if ((sscanf(sword, " (' %[^'\n] ') ", tokens) == 1) || (sscanf(sword, " ' %[^'\n] ' ", tokens) == 1))
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

      // FINISH SO NOT GOES BACK TO FIND CLASS
      //if (force_next_line == 1) { break; }

      sword = strtok(NULL, str_delim);
    }
  }
}

// ADD LEXER TO CHECK IF WORTH TO PARSE TODO
// CURRENTLY ADDING VARIABLES
void *lex_class(char toks[100][100])
{
  for (i=0;i<idx;i++)
  {
    printf("--> %s\n", toks[i]);
  }

  if (strcmp(*toks, "@") == 0)
  {
    printf("VARIABLE\n");
  }
  else 
  {
    printf("ONLY VARIABLES IN CLASS\n");
    exit(1);
  }

  // ## TRY SSCANF LATER ##
  // if (sscanf(*toks, " @ %[^'\n] ", variables) == 1)
  // {
  //   printf("%s \n", variables);  
  //   //str_delim = str_delim_def;
  //   //state = FIND_ALL;
  // }

}

