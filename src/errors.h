/*
File for all errors

template:

if (in == X){printf("X\n");}

*/

void ERROR_FOUND(int in)
{
  if (in == 1){printf("USE NOCLASS IF NOTHING IN CLASS\n");}
  if (in == 2){printf("ONLY VARIABLES IN CLASS\n");}
  if (in == 3){printf("NO EQUALS SYMBOL\n");}
  if (in == 4){printf("Missing Equal Symbol\n");}
  if (in == 5){printf("Need A Class To Use Variables\n");}
  if (in == 6){printf("No Variable Exists with that name\n");}
  if (in == 7){printf("Warning Assigned Variable(s) in class not used\n");}
}
