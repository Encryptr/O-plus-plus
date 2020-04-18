#include <stdio.h>
#include "lexer.h"

int main()
{
	struct Opp_Scan data = {0};
	opp_init_lex(&data, "hello 123 + - \"hello world im\" 1.2");
	dump_tokens(&data);
}