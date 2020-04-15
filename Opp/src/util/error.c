#include "error.h"

void opp_error(struct Opp_Scan* s, const char* str, ...)
{
	printf("[%u] " , s->line);
	va_list ap;
	va_start(ap, str);
	vfprintf(stdout, str, ap);
	fprintf(stdout, "\n");
	va_end(ap);
	exit(2);
}

void internal_error(const char* str, int code)
{
	printf("* ERROR * %d ", code);
	printf("%s\n", str);
	exit(code);
}
