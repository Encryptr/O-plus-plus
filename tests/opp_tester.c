#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>

int main() 
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

	return 0;
}