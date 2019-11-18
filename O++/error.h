const char* ERROR_LIST[] = {
	"Expected string after 'import'",
	"Expected true or false expression after 'if'",
	"Expected condtion after 'if'",
	"No else case in 'if' statment",
	"Invalid type in 'print'",
	"No terminating \'"
};

void syntax_error(struct Scan *s, char* og)
{
	printf("[%ld] Syntax Error %s\n", s->line, og);
	exit(1);
}

void expected_error(struct Scan *s, char* og, const char* exp)
{
	printf("[%ld] Expeceted %s not %s\n", s->line, exp, og);
	exit(1);
}

void invalid(struct Scan *s)
{
	printf("[%ld] Invlaid Type\n", s->line);
	exit(1);
}

void PRINT_ERROR(int error_id)
{
	printf("%s\n", ERROR_LIST[error_id]);
	exit(error_id);
}
