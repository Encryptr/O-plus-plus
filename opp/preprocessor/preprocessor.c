#include "preprocessor.h"
#include <stdio.h>
#include <stdlib.h>

static char path_buffer[32] = {0};

const char* opp_invoke_preproc(const char* path)
{
	char sys_buf[64] = {0};
	int index = 0;
	const char* i = path;

	while (*i != '.')
		path_buffer[index++] = *i++;
	path_buffer[index] = 0;

	sprintf(path_buffer, "%s_dumped.c", path_buffer);
	sprintf(sys_buf, "cpp -std=c89 -P %s > %s", path, path_buffer);
	system(sys_buf);

	return path_buffer;
}

void opp_cleanup(const char* path)
{
	char sys_buf[64] = {0};
	sprintf(sys_buf, "rm %s", path);
	system(sys_buf);
}