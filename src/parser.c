#include "o++.h"
#include "function.c"

void main_lex(char intake[1000])
{
    str_delim = str_delim_def;
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
          // TODO: START THINKING OF HOW TO IMPLEMENT COMPILER CLASS
          if (strcmp(sword, "class") == 0)
          {
            has_class = 1;
            state = COPY_CONT;
          }
          else if (strcmp(sword, "noclass") == 0)
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
          if (strcmp(sword, "end") == 0)
          {
            lex_class(class_tokens);

            state = FIND_ALL;
          }
          strcpy(class_tokens[idx], sword);
          idx++;
        break;

        case FIND_ALL:
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
          else if (strcmp(sword, "endif") == 0)
          {
            if (endif_amount >= 1)
            {
              // TODO: ADD IF ENDIF_AMOUNT NOT MATCHED UP TO IF
              endif_amount--;
            }
            else {ERROR_FOUND(14); exit(1);}
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
        // TODO: MOVE ALL INTO FUNCTION
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
          state_if_count++;
          // TODO: ADD ERROR NOT COMPLETE IF STATMENT using if_step var
          if (if_result == 2)
          {
            endif_amount++;
            state = FIND_ALL;

          }
          if (if_result == 1)
          {
            if (strcmp(sword, "endif") == 0)
            {
              state = FIND_ALL;
            }
            sword = "";
          }
        break;

      }

      sword = strtok(NULL, str_delim);
   }
}
