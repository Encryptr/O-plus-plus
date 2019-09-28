#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//#include "../tengine.h"
#include "o++.h"
// #include "lexer.c"
// #include "parser.c"

int main(int argc, char **argv)
{

	if (argc == 2)
	{
		init_opp(argv[1]);
	}
	else printf("NO FILE!!\n"), exit(1);


	return 0;
}