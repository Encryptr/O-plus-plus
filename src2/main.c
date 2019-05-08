#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "o++.h"
#include "lexer.c"

/*

Version 2.0 O++ Interpreter

*/

int main(int argv, char** argc)
{
	OPP opp;
	long size;
	char *buffer;
	FILE *f;

	if (argv == 2)
	{
		f = fopen(argc[1], "rb");
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		rewind(f);

		buffer = calloc(1, size + 1);
		if(!buffer)
		{
			fclose(f);
			printf("ERROR CALLOC\n");
			exit(1);
		}

		if(1 != fread(buffer, size, 1, f))
		{
			fclose(f); free(buffer); printf("ERROR\n"); exit(1);
		}

		init(buffer, &opp);
		main_lex(&opp);
		printf("%s\n", opp.src);


		fclose(f);
		free(buffer);
	}
}