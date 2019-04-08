#ifndef OPP
#define OPP

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "o++.h"
//#include "lex.c"

#define MAX_LENGTH 1000

Tok t = VAR;
IFSTATE ifstate = CHECK_VAR;
Variable vv;
Variable *vptr = &vv;
COMPARE_IF ci;
COMPARE_IF *cif = &ci;

States state = FIND_CLASS;


void lex_class(char toks[100][100]);
void print_var(const char *varname);
// int if_statment(const char *comp, char *curr);
int if_statment(const char *comp);
//void print_var(const char *varname);



int main(int argc, char *argv[])
{
  FILE *file;

  file = fopen(argv[1], "rt");
  if (file == NULL)
  {
    printf("NO FILE NAMED %s\n", argv[1]);
    return -1;
  }

  str_delim = str_delim_def;
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

          if (strcmp(sword, "class") == 0)
          {
            has_class = 1;
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
            lex_class(class_tokens);

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
          else if (strcmp(sword, "if") == 0)
          {
            if (has_class == 1)
            {
                state = IF_STATE;
            }
            else {ERROR_FOUND(10); exit(1);}
          }
          else if (strcmp(sword, "") == 0)
          {
            state = FIND_ALL;
          }
          else
          {
            printf("NOT VALID -> %s\n", sword);
            exit(1);
          }
        break;

        case PRINT_TOK:
        if ((sscanf(sword, " ' %[^'\n] ' ", print_string) == 1))
        {
          printf("%s \n", print_string);
          str_delim = str_delim_def;
          state = FIND_ALL;
        }
        // Inplement Printing Variables
        else if ((sscanf(sword, " @%s ", print_variable[vars]) == 1))
        {
          if (has_class == 1)
          {
            print_var(print_variable[vars]);
            vars++;
            str_delim = str_delim_def;
            state = FIND_ALL;
          }
          else
          {
            ERROR_FOUND(5);
            exit(1);
          }
        }
        else
        {
          printf("SYNTAX PRINT ERROR -> %s \n", sword);
          exit(1);
        }
        break;

        case IF_STATE:
          // Assing to int later to compare if should go on or now or else it will still stay at same postion
          // printf("RESULT: %d\n", if_statment(sword));
          // if_statment(sword);
          // state_if_count++;

          state = FIND_ALL;
        break;

      }

      sword = strtok(NULL, str_delim);
    }
   }
}


void lex_class(char toks[100][100])
{

  if (strcmp(*toks, "") == 0)
  {
    ERROR_FOUND(1);
    exit(1);
  }

  for (i=0;i<idx;i++)
  {
    //printf("--> %s\n", toks[i]);

    switch (t)
    {
      case VAR:
        if (sscanf(toks[i], "@%s", vptr->var_name[var_count]) == 1)
        {
          // printf("%s\n", vptr->var_name);
          // i is always one less than idx
          // TODO Make quit with error if switchcannot be completed
          //printf("%d | %d\n", i,idx);
          t = EQ;

        }
        else
        {
          ERROR_FOUND(2);
          exit(1);
        }
      break;

      case EQ:
        if (strcmp(toks[i], ":") == 0)
        {
          //printf("EQUALS FOUND\n");
          t = TYPE;
        }
        else {ERROR_FOUND(3); exit(1);}

      break;

      case TYPE:
        // NO TYPE IF FILE THROW ERROR IF NO TYPE FOUND
        //printf("HERE AT TYPE\n");
        if (*toks[i] >= '0' && *toks[i] <= '9')
        {
          int num = atoi(toks[i]);
          vptr->val[var_count] = num;
          var_count++;
        }
        else
        {
          printf("Var is not equal to a Number: %s\n", toks[i]);
        }

        t = VAR;

      break;

    }

  }

  // Check if var names and values are alligned
  // for (i=0;i<var_count;i++)
  // {
  //   printf("Name: %s Value: %d\n", vptr->var_name[i], vptr->val[i]);
  // }

}
//---------------------------------------------------
//---------------------------------------------------
void print_var(const char *varname)
{
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
//---------------------------------------------------
//---------------------------------------------------

int if_statment(const char *comp)
{
  // while (comp != NULL)
  // {
    switch (ifstate)
    {
      case CHECK_VAR:
        if (sscanf(comp, "@%s", cif->comp_var[state_if_count]) == 1)
        {
          for (a=0;a<var_count;a++)
          {
            // FIX CHANGING STATE PROBLEM
            if (strcmp(cif->comp_var[state_if_count], vptr->var_name[a]) == 0)
            {
              //printf("ITS THE SAME\n");
              ifstate = EQEQ;
              break;
            }
          }
          if (ifstate != EQEQ) {
            ERROR_FOUND(8); printf("-> %s\n", comp); exit(1);
          }
        }
        else {ERROR_FOUND(9); printf("-> %s\n", comp); exit(1);}
      break;

      case EQEQ:
        if (strcmp(comp, "::") == 0)
        {
          // printf("READY TO CHECK NUM\n");
          ifstate = NUM_CHECK;
        }
        else {ERROR_FOUND(3); printf("-> %s\n", comp); exit(1);}
      break;

      case NUM_CHECK:
        if (*comp >= '0' && *comp <= '9')
        {
          int value = atoi(comp);
          cif->comp_val[state_if_count] = value;
          //printf("COMP VAR: %s ITS VALUE: %d\n", cif->comp_var[state_if_count], cif->comp_val[state_if_count]);
        }
        for (a=0;a<var_count;a++)
        {
            if (cif->comp_val[state_if_count] == vptr->val[a])
            {
              printf("Same NUM\n");
              return 2;
              break;
            }
        }
        printf("NOT SAME\n");
        return 1;
      break;

    }
  // ERROR_FOUND(11); exit(1);
}

#endif
