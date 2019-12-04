#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// INCLUDE ALL HEADERS

// enum Flags {
// 	NONE, NO_WARNING, DUMP_TOKS
// };

// enum Flags flags = NONE;

// void help_menu()
// {
// 	system("clear");
// 	printf("\n\x1b[32mO++ Programming Language\x1b[0m\n");
// 	printf("Written by Maksymilian Sawoniewicz\n");
// 	printf("\n\x1b[31mFormat: o++ [flag] [file]\x1b[0m\n");
// 	printf("\n-h    | For help menu\n");
// 	printf("-dump | For dumping file tokens\n\n\n");
// 	exit(1);
// }

// void inter_options(char** option, int amount)
// {
// 	for (int i=1;i<amount;i++)
// 	{
// 		if (!strcmp(option[i], "-nowarn"))
// 		{
// 			flags = NO_WARNING;
// 		}
// 		else if (!strcmp(option[i], "-dump"))
// 		{
// 			flags = DUMP_TOKS;
// 		}
// 		else {
// 			printf("Not a O++ flag... [%s]\n", option[i]);
// 		}
// 	}
// }

// void init_file(const char* fname, struct Scan *d, char* content)
// {
// 	long size;
// 	FILE *file;

// 	file = fopen(fname, "r");
// 	if (!file)
// 		printf("[%s] Is not a file...\n", fname),exit(1);
// 	fseek(file, 0, SEEK_END);
// 	size = ftell(file);
// 	rewind(file);
// 	content = calloc(1, size + 1);

// 	fread(content, size, 1, file);
// 	fclose(file);

// 	init_lex(d, content);
// }

// void init_opp(const char* fname, struct Obj* root)
// {
// 	struct Scan data = {0};
// 	char* content = NULL;

// 	init_file(fname, &data, content);
// 	preprocessor(&data);

// 	root = analize(&data);

// 	while (root->type != NIL)
// 	{
// 		eval(root);
// 		root = root->cdr;
// 	}

// 	if (flags != NO_WARNING)
// 		warning_dump(&data);

// 	free(content);
// }

int main(int argc, char** argv)
{
	struct Obj* cell;

	map = createMap();

	if (argc==2)
		init_opp(argv[1], cell);
	else if (argc > 2)
	{
		int len = 0;
		while (argv[len] != NULL)
			len++;
		inter_options(argv, len-1);
		init_opp(argv[len-1], cell);
	}
	else
		help_menu();

	free_table(map);

	return 0;
} 