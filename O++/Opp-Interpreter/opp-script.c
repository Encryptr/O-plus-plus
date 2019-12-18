#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lexer.h"
#include "hashmap.h"
#include "opp-script.h"
#include "ast.h"
#include "parser.h"

enum Flags {
	NONE, NO_WARNING, DUMP_TOKS,
	AST_PRINT
};

enum Flags flags = NONE;

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

void inter_options(char** option, int amount)
{
	for (int i=1;i<amount;i++)
	{
		if (!strcmp(option[i], "-nowarn"))
			flags = NO_WARNING;
		else if (!strcmp(option[i], "-dump"))
			flags = DUMP_TOKS;
		else if (!strcmp(option[i], "-ast"))
			flags = AST_PRINT;
		else 
			printf("Not a O++ flag... [%s]\n", option[i]);
	}
}

void init_file(const char* fname, struct Scan *s, char* content)
{
	long size;

	s->file = fopen(fname, "r");
	if (!s->file)
		printf("[%s] Is not a file...\n", fname),exit(1);
	fseek(s->file, 0, SEEK_END);
	size = ftell(s->file);
	rewind(s->file);
	content = calloc(1, size + 1);

	fread(content, size, 1, s->file);
	fclose(s->file);

	init_lex(s, content);
}

void init_opp(const char* fname)
{
	struct Scan data = {0};
	struct Opp_Ast oppTree = {0};

	char* content = NULL;

	init_file(fname, &data, content);
	preprocessor(&data);
		
	if (flags == DUMP_TOKS)
		dump_tokens(&data);
	// oppTree.tree = opp_parser_init(&data);
	else opp_init_parser(&data);
	
	free(content);
}

int main(int argc, char** argv)
{
	map = createMap();

	if (argc==2)
		init_opp(argv[1]);
	else if (argc > 2)
	{
		int len = 0;
		while (argv[len] != NULL)
			len++;
		inter_options(argv, len-1);
		init_opp(argv[len-1]);
	}
	else
		help_menu();

	free_table(map);

	return 0;
} 