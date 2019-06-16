#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "morse.h"
#include "lexer.c"
#include "parser.c"
#include "gen_asm.c"


int main(int argc, char **argv)
{
	Morse morse;
	Parser par;
	Variable va;
	Gen_Asm ga;
	long size;

	FILE *file;

	if (argc == 2)
	{
		file = fopen(argv[1], "r");
		if (!file)
		{
			printf("NOT A FILE!!\n");
			exit(1);
		}
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		rewind(file);
		content = calloc(1, size + 1);

		fread(content, size, 1, file);
		fclose(file);

		lex_init(&morse, content);
		preprocessor(&morse);
		while (*morse.src)
		{
			interpret(&morse);
			morse.src++;
		}
		parse_begin(&morse, &par, &va, &ga);

		free(content);
	}
	else if (argc == 4)
	{
		if (strcmp(argv[2], "-c") ==0)
		{
			isC = true;
			file = fopen(argv[1], "r");
			if (!file)
			{
				printf("NOT A FILE!!\n");
				exit(1);
			}
			fseek(file, 0, SEEK_END);
			size = ftell(file);
			rewind(file);
			content = calloc(1, size + 1);

			fread(content, size, 1, file);
			fclose(file);

			lex_init(&morse, content);
			preprocessor(&morse);
			while (*morse.src)
			{
				interpret(&morse);
				morse.src++;
			}
			begin_gen(&ga,argv[3]);
			parse_begin(&morse, &par, &va, &ga);

			free(content);
		}
	}
	else printf("NO FILE!!\n"),exit(1);


	return 0;
}