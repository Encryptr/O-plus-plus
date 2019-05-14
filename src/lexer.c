#include "o++.h"
#include "parser.c"
#include "reuse.c"

void main_lex(char intake[1000])
{
    str_delim = str_delim_def;
    char *sword = strtok(intake, str_delim);

    while (sword != NULL)
    {
      //printf("[%s] state=%d\n", sword, state);

      switch (state)
      {
        case IGNORE:
        // WORK ON COMMENT ERROR WITH VARIABLES
          if (strcmp(sword, stdlib[7]) == 0)
          {
            state = FIND_ALL;
          }
          sword = "";
        break;

        case COPY_CONT:
          if (strcmp(sword, stdlib[1]) == 0)
          {
            state = FIND_ALL;
          }
        break;

        case FIND_ALL:
          if (strcmp(sword, "class") == 0)
          {
            state = COPY_CONT;
          }
          else if (strcmp(sword, stdlib[6]) == 0)
          {
            state = IGNORE;
          }
          //--------------------------------------------------------------------
          else if (strcmp(sword, stdlib[5]) == 0)
          {
            str_delim = "\n";
            state = PRINT_TOK;
          }
          //--------------------------------------------------------------------
          else if (sscanf(sword, "@%s", pva->gvar[pva->gvar_idx]) == 1)
          {
            state = CHANGE_VAR;
          }
          else if (sscanf(sword, "( @%[^)\n] )", pva->gvar[pva->gvar_idx]) == 1)
          {
            
            call_func(pva->gvar[pva->gvar_idx]);
            
          }
          //--------------------------------------------------------------------
          //--------------------------------------------------------------------
          else if (strcmp(sword, stdlib[3]) == 0)
          {
            if (has_class == 1)
            {
                if_counter++;
                state = IF_STATE;
            }
            else {ERROR_FOUND(10); exit(1);}
          }
          else if (strcmp(sword, stdlib[4]) == 0)
          {
            if_counter--;
            state = FIND_ALL;
          }
          //--------------------------------------------------------------------
          else if (strcmp(sword, "") == 0)
          {
            state = FIND_ALL;
          }
          else
          {printf("NOT VALID -> %s\n", sword); exit(1);}
          //--------------------------------------------------------------------
        break;

        case PRINT_TOK:
        // TODO: FIX PRINT 
        if ((sscanf(sword, " ' %[^'\n] ' ", print_string) == 1))
        {
          printf("%s \n", print_string);
          str_delim = str_delim_def;
          state = FIND_ALL;
        }
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
        // else if (sscanf(sword, "#%s", print_string[ascii_amount]) == 1)
        // {
        //   // NEVER FINISHED "," FINISH IF WANT!!
        //   int res = atoi(print_string[ascii_amount]);
        //   putchar(res);
        //   ascii_amount++;
        //   state = FIND_ALL;
        // }
        else
        {
          // REMOVE IF WANT!!
          printf("\n");
          printf("SYNTAX PRINT ERROR -> %s \n", sword);
          exit(1);
        }
        break;

        case IF_STATE:
          if_result = if_statment(sword);
          if (if_result == 2)
          {
            state_if_count++;
            state = FIND_ALL;
          }
          if (if_result == 1)
          {
            state_if_count++;
            endif_counter = if_counter;
            state = ENDIF_FIND;
          }
        break;

        case ENDIF_FIND:
          if (strcmp(sword, "endif") == 0)
          {
            if (endif_counter == if_counter) {state = FIND_ALL;}
            if_counter--;
          }
          else if (strcmp(sword, "if") == 0) { if_counter++; }
        break;

        case CHANGE_VAR:
        if (strcmp(sword, "->") == 0)
        {
          strcpy(pfv->fvar[pfv->fvar_amount],pva->gvar[pva->gvar_idx]);
          // IDX 
          pva->gvar_idx++;
          state = FUNC;
          break;
        }
        else
        {
          if (cpy_keep == 0)
          {
            strcpy(pcv->change_comp_var[change_count],pva->gvar[pva->gvar_idx]);
            pva->gvar_idx++;
            cpy_keep = 1;
          }
          for (pcv->which_match=0;pcv->which_match<var_count;pcv->which_match++)
          {
            if (strcmp(pcv->change_comp_var[change_count], vptr->var_name[pcv->which_match]) == 0)
            {
              pcv->does_match = 1;
              break;
            }
          }
          if (pcv->does_match == 0)
          {
            ERROR_FOUND(6);
            printf("->%s\n", pcv->change_comp_var[change_count]);
            exit(1);
          }
          pcv->does_match = 0;

          change_variable(sword);

          if (pcv->cont == 1)
          {
            cpy_keep = 0;
            pcv->cont = 0;
            state = FIND_ALL;
          } 
        }      
        //else {ERROR_FOUND(20); printf("->%s\n", sword); exit(1);}

        break;

        case FUNC:

          create_func(sword);
          if (done_func == 1)
          {
            done_func = 0;
            state = FIND_ALL;
            break;
          }

        break;
      }

      sword = strtok(NULL, str_delim);
   }
}
