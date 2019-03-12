/*
Super Simple C Interpreter

Current Tasks = Tasks.txt



--> By: Maks Sawoniewicz <--

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_LENGTH 2048
#define IF_STRCMP(a,b) if (strcmp(a, b) == 0)

void error();
void parse();

char tokens[255][255] = {};
int idx = 0;

int main (int argc, char *argv[])
{
	
	FILE *fin;

	fin = fopen(argv[1], "rt");
	if (fin == NULL) 
	{
		printf("NO FILE %s\n", argv[1]);
		return -1;
	}
	for(;;)
	{

		char tok[2048];
		fscanf(fin, " %s \n", tok);

		IF_STRCMP(tok, "print")
		{
			strcpy(tokens[idx], "PRINT");
			idx++;
			continue;
			//continue
		}
		IF_STRCMP(tok, "@")
		{
			strcpy(tokens[idx], "VARIABLE");
			idx++;  
			break;
		}
		else
		{
			printf("\"%s\" Not a function\n", tok);
			break;
		}
	}

	//return tokens;
	parse();
}

void parse()
{
	for (int i=0; i < idx; i++)
	{
		printf("%s\n", tokens[i]);
	}
}
