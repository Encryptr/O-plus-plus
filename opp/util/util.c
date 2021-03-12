#include "util.h"
#include "../os/os.h"
#include "../opp.h"
#include <setjmp.h>

void opp_error(struct Opp_Scan* s, const char* str, ...)
{
	va_list ap;
	va_start(ap, str);
	
	printf("%s:%u:%u ", s->io.fname, s->line, s->colum);
	#ifdef UNX 
	printf(CL_RED);
	#endif

	printf("error: \n\t");
	printf(CL_RESET); 
	vfprintf(stdout, str, ap);
	fprintf(stdout, "\n");
	va_end(ap);

	longjmp(global_state.error_buf, OPP_ERROR);
}

void internal_error(const char* str)
{
	#ifdef UNX 
	printf(CL_RED); 
	#endif 
	printf("** Error: %s **\n", str); 
	#ifdef UNX 
	printf(CL_RESET); 
	#endif 

	longjmp(global_state.error_buf, OPP_ERROR);
}