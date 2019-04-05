#ifndef LEX
#define LEX

//#include "o++.c"

// #### SEPARATE LEX FUNCTIONS INTO SEPRATE FILES
//

// #include "o++.h"
//
// void main_lex()
// {
//   char *sword = strtok(fline, str_delim);
//
//   while (sword != NULL)
//   {
//     //printf("[%s] state=%d\n", sword, state);
//
//     switch(state)
//     {
//
//       case IGNORE:
//
//         if (strcmp(sword, ">") == 0)
//         {
//           state = FIND_ALL;
//         }
//         sword = "";
//       break;
//
//       case FIND_CLASS:
//
//         if (strcmp(sword, "class") == 0)
//         {
//           has_class = 1;
//           state = COPY_CONT;
//         }
//         else if (strcmp(sword, "noclass") == 0)
//         {
//           //printf("NO CLASS continuing...\n");
//           state = FIND_ALL;
//         }
//         else
//         {
//           printf("CLASS DECLARATION MISSING\n");
//           exit(1);
//         }
//       break;
//
//       case COPY_CONT:
//         if (strcmp(sword, "end") == 0)
//         {
//           lex_class(class_tokens);
//
//           state = FIND_ALL;
//         }
//         strcpy(class_tokens[idx], sword);
//         idx++;
//       break;
//
//       case FIND_ALL:
//         // ## COMMENTS WORKING ##
//         if (strcmp(sword, "<") == 0)
//         {
//           //printf("FOUND COMMENT\n");
//           state = IGNORE;
//         }
//         else if (strcmp(sword, "print") == 0)
//         {
//           str_delim = "\n";
//           state = PRINT_TOK;
//         }
//         else if (strcmp(sword, "") == 0)
//         {
//           state = FIND_ALL;
//         }
//         else
//         {
//           printf("SYNTAX ERROR %s\n", sword);
//           exit(1);
//         }
//       break;
//
//       case PRINT_TOK:
//       if ((sscanf(sword, " ' %[^'\n] ' ", print_string) == 1))
//       {
//         printf("%s \n", print_string);
//         str_delim = str_delim_def;
//         state = FIND_ALL;
//       }
//       // Inplement Printing Variables
//       else if ((sscanf(sword, " @%s ", print_variable[vars]) == 1))
//       {
//         if (has_class == 1)
//         {
//           print_var(print_variable[vars]);
//           vars++;
//           str_delim = str_delim_def;
//           state = FIND_ALL;
//         }
//         else
//         {
//           ERROR_FOUND(5);
//           exit(1);
//         }
//       }
//       else
//       {
//         printf("SYNTAX PRINT ERROR %s \n", sword);
//         exit(1);
//       }
//       break;
//
//     }
//
//     sword = strtok(NULL, str_delim);
//   }
// }
//
//
//
//
//
//

#endif
