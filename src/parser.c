#include "o++.h"
#include "function.c"

void main_lex(char intake[1000], FILE *fi)
{
    str_delim = str_delim_def;
    char *sword = strtok(fline, str_delim);


    while (sword != NULL)
    {
      //printf("[%s] state=%d\n", sword, state);

      switch (state)
      {

        case IGNORE:
          if (strcmp(sword, stdlib[7]) == 0)
          {
            if (back_to_class == 1) { back_to_class = 0; state = FIND_CLASS; }
            else state = FIND_ALL;
          }
          sword = "";
        break;

        case FIND_CLASS:
          // TODO: ADD INT MAIN CLASSES
          if (strcmp(sword, stdlib[6]) == 0)
          {
            back_to_class = 1;
            state = IGNORE;
          }
          else if (strcmp(sword, stdlib[0]) == 0)
          {
            has_class = 1;
            state = COPY_CONT;
          }
          else if (strcmp(sword, stdlib[2]) == 0)
          {
            state = FIND_ALL;
          }
          else
          {
            printf("CLASS DECLARATION MISSING\n");
            exit(1);
          }
        break;

        case COPY_CONT:
          if (strcmp(sword, stdlib[1]) == 0)
          {
            lex_class(class_tokens);

            state = FIND_ALL;
          }
          strcpy(class_tokens[idx], sword);
          idx++;
        break;

        case FIND_ALL:
          if (strcmp(sword, stdlib[6]) == 0)
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
          else if (sscanf(sword, "@%s", pcv->change_comp_var[change_count]) == 1)
          {
            if (has_class == 1)
            {
                state = CHANGE_VAR;
            }
            else {ERROR_FOUND(17); exit(1);}
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
        // TODO: Add printing @var + @var
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
        else if (sscanf(sword, "#%s", print_string[ascii_amount]) == 1)
        {
          // NEVER FINISHED "," FINISH IF WANT!!
          int res = atoi(print_string[ascii_amount]);
          putchar(res);
          ascii_amount++;
          state = FIND_ALL;
        }
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
          pcv->cont = 0;
          state = FIND_ALL;
        }
        break;

      }

      sword = strtok(NULL, str_delim);
   }
}
