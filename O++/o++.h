#ifndef OPPH
#define OPPH

#define LENGTH 1000
#define SMALL 1000
#define MAX 10000

#include "error.c"
#include "lexer.c"
#include "parser.c"

// void lex_init(Morse *m, char* source);
// void preprocessor(Morse *m);

void init_opp(const char* fil)
{
	Morse morse = {0};
	Parser par = {0};
	Variable va = {0};
	long size;

	FILE *file;
	file = fopen(fil, "r");
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
	parse_begin(&morse, &par, &va);

	free(content);
}


#endif