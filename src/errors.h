/*
File for all errors

template:

if (in == X){printf("X\n");}

*/


// #define ERROR_FOUND(in) \
//   printf("ERROR ln=%d -> ", __LINE__); ERROR_FOUNDx(in);

void ERROR_FOUND(int in)
{
  if (in == -1){printf("NOT VALID\n");}
  if (in == 1){printf("USE noclass IF NOTHING IN CLASS\n");}
  if (in == 2){printf("ONLY VARIABLES IN CLASS\n");}
  if (in == 3){printf("NO EQUALS SYMBOL OR NO SPACE\n");}
  if (in == 4){printf("Missing Equal Symbol\n");}
  if (in == 5){printf("Need A Class To Use Variables\n");}
  if (in == 6){printf("No Variable Exists with that name\n");}
  if (in == 7){printf("Warning Assigned Variable(s) in class not used\n");}
  if (in == 8){printf("Variables provided are not the same\n");}
  if (in == 9){printf("Missing Variable to Compare\n");}
  if (in == 10){printf("Need A Class To Use If Statments\n");}
  if (in == 11){printf("Not A Complete If Statment\n");}
  if (in == 12){printf("Uncomplete Variable Declaration\n");}
  if (in == 13){printf("Variable Already Declared with the name\n");}
  if (in == 14){printf("Unexpected ENDIF or amount of if != endif\n");}
  if (in == 15){printf("Variable equal to SYNTAX ERROR\n");}
  if (in == 16){printf("Only compare variables to Numbers\n");}
  if (in == 17){printf("Need Class to use Variables\n");}
  if (in == 18){printf("Not a valid operator\n");}
  if (in == 19){printf("NOT VALID COMPERESON\n");}
}
