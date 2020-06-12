#include "../opp.h"
#include "../parser/parser.h"
#include "../env/enviroment.h"
#include "../interpreter/interpreter.h"
#include "../parser/ostdlib.h"

enum Opp_Flag { NONE, DUMP } Flag;

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

	if (Flag == DUMP) {
		dump_tokens(&data);
		return;
	}

	parser = opp_parse_init(&data);
	parser->mode = IFILE;
	opp_init_environment();
	opp_init_std();

	opp_eval_init(parser);
}