#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>

struct Ctx {
	FILE* in;
	char* src;
};

typedef struct Ctx Ctx;

Ctx ctx;


void find_test()
{

}

void open_file(char* fname)
{
	ctx.in = fopen(fname, "r");

	if (!ctx.in)
		puts("Error opening file..."), exit(0);

	// read source
}

void read_dir()
{
	DIR *d;
	struct dirent *dir;
	
	d = opendir(".");
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if (dir->d_type == DT_REG && 
				strstr(dir->d_name, ".opp") != NULL) 
			{ 
				printf("%s\n", dir->d_name);
			}
		}
		closedir(d);
	}
}

void help()
{
	puts("Opp Test Module");
}
 
int main(int argc, const char** argv)
{
	if (argc < 2)
		read_dir();

	return 0;
}
