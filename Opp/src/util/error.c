#include "error.h"

void opp_error(struct Opp_Scan* s, const char* str, ...)
{
	if (s != NULL) {
		#ifdef UNX
		printf(CL_RED);
		printf("[%u] " , s->line);
		printf(CL_RESET);
		#else 
		printf("[%u] " , s->line);
		#endif
	} 

	va_list ap;
	va_start(ap, str);
	vfprintf(stdout, str, ap);
	fprintf(stdout, "\n");
	va_end(ap);
	exit(2);
}

void internal_error(const char* str, int code)
{
	#ifdef UNX
	printf(CL_RED);
	printf("* ERROR * %d ", code);
	printf(CL_RESET);
	#else
	printf("* ERROR * %d ", code);
	#endif
	printf("%s\n", str);
	exit(code);
}

void opp_warning(struct Opp_Scan* s, const char* str, ...)
{
	va_list ap;
	va_start(ap, str);
	if (s != NULL) 
	{
		#ifdef UNX
		printf(CL_YELLOW);
		printf("* WARNING *");
		vfprintf(stdout, str, ap);
		fprintf(stdout, "\n");
		printf(CL_RESET);
		#else 
		printf("* WARNING *");
		vfprintf(stdout, str, ap);
		fprintf(stdout, "\n");
		#endif
	}
	else 
	{
		#ifdef UNX
		printf(CL_YELLOW);
		printf("[%u] * WARNING *", s->line);
		vfprintf(stdout, str, ap);
		fprintf(stdout, "\n");
		printf(CL_RESET);
		#else 
		printf("[%u] * WARNING *", s->line);
		vfprintf(stdout, str, ap);
		fprintf(stdout, "\n");
		#endif
	}

	va_end(ap);
}
