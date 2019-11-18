#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// #include "main.h"
#include "lexer.c"
#include "error.h"
#include "parser.c"
#include "ast.c"

void help_menu()
{
	system("clear");
	printf("\n\x1b[32mO++ Programming Language\x1b[0m\n");
	printf("Written by Maksymilian Sawoniewicz\n");
	printf("\n\x1b[31mFormat: o++ [flag] [file]\x1b[0m\n");
	printf("\n-h    | For help menu\n");
	printf("-dump | For dumping file tokens\n\n\n");
	exit(1);
}

void init(const char* fname, struct Obj* root)
{
	struct Scan data = {0};
	char* content = NULL;
	long size;
	FILE *file;

	file = fopen(fname, "r");
	if (!file)
		printf("[%s] Is not a file...\n", fname),exit(1);
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);
	content = calloc(1, size + 1);

	fread(content, size, 1, file);
	fclose(file);

	init_lex(&data, content);
	preprocessor(&data);

	root = analize(&data);

	while (root->type != NIL)
	{
		eval(root);
		root = root->cdr;
	}

	free(content);
}

int main(int argc, const char** argv)
{
	struct Obj* cell;
	if (argc==2)
		init(argv[1], cell);
	else
		help_menu();

	return 0;
} 