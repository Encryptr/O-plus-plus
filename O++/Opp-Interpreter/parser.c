#include "parser.h"

struct Opp_Node* opp_parse_num(struct Scan* s)
{
	struct Opp_Node* node;

	int number = atoi(s->lexeme);
	struct Opp_Value value = {.ival = number};

	node = opp_make(AST_INTEGER, value);
	node->rt = opp_parser(s);

	return node;
}

struct Opp_Node* opp_parse_fend(struct Scan* s)
{
	struct Opp_Node* node;

	node = opp_make(AST_END, null_opp_value());

	return node;
}

struct Opp_Node* opp_parse_nil(struct Scan* s)
{
	struct Opp_Node* node;

	node = opp_make(AST_STMT_END, null_opp_value());

	return node;
}

struct Opp_Node* opp_parse_stmt_start(struct Scan* s)
{
	struct Opp_Node* node;

	struct Opp_Value value = {0};
	value.strval = malloc(sizeof(char)*strlen(s->lexeme));
	strcpy(value.strval, s->lexeme);

	node = opp_make(AST_STMT, value);
	node->lt = opp_parser(s);
	node->rt = opp_parser(s);

	return node;
}

struct Opp_Node* opp_parser_init(struct Scan* s)
{
	struct Opp_Node* tree;
	tree = (struct Opp_Node*)malloc(sizeof(struct Opp_Node));
	// tree->type = OPP_AST_START;
	
	tree = opp_parser(s);

	return tree;
} 

struct Opp_Node* opp_parser(struct Scan* s)
{
	struct Opp_Node* node;

	for (;;)
	{
		next(s);

		switch (s->tok)
		{
			case FEND:
			{
				node = opp_parse_fend(s);
				return node;
			}
			case SEMICOLON:
			{
				node = opp_parse_nil(s);
				return node;
			}
			case NUM:
			{
				node = opp_parse_num(s);
				return node;
			}

			case IDENT: case TFUNC: case TVAR:
			{
				node = opp_parse_stmt_start(s);
				return node;
			}
		}

	}
}
