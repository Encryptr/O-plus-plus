#include <stdio.h>
#include "opp.h"
#include "./lexer/lexer.h"
#include "./parser/parser.h"
#include "./env/enviroment.h"
#include "./interpreter/interpreter.h"
#include "./parser/ostdlib.h"

static void help_menu()
{
	#ifdef UNX
		system("clear");
	#else
		system("cls");
	#endif
	printf("\n\x1b[32mOLisp Programming Language\x1b[0m\n");
	printf("Written by Maks S\n");
	printf("\n\x1b[31mFormat: o++ [flag] [file]\x1b[0m\n");
	printf("\n-h    | For help menu\n");
	printf("-dump | For dumping file tokens\n\n\n");
	exit(1);
}

void init_file(const char* fname, struct Opp_Scan *s)
{
	long size;

	s->file = fopen(fname, "r");
	if (!s->file)
		printf("[%s] Is not a file...\n", fname), exit(1);
	fseek(s->file, 0, SEEK_END);
	size = ftell(s->file);
	rewind(s->file);
	char *content = calloc(1, size + 2); 

	fread(content, size, 1, s->file);
	fclose(s->file);

	opp_init_lex(s, content);
}

void init_opp(const char* fname)
{
	struct Opp_Scan data = {0};
	struct Opp_Parser* parser;

	init_file(fname, &data);
	parser = opp_parse_init(&data);
	opp_init_environment();
	opp_init_std();

	opp_eval_init(parser);
}

int main(int argc, char** argv)
{
	if (argc == 2)
		init_opp(argv[1]);
	else 
		help_menu();

	return 0;
} 