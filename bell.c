#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LENGTH 100

char line_in[MAX_LENGTH];

typedef enum {
	FIND_INSTR,
	FIND_ARG,
} States;


int main()
{
    for(;;)
    {
        printf("bell> ");

        while (gets(line_in) == NULL) {
            printf("ERROR input\n");
        }

	/* old for file 
	while (fgets(line_in, MAX_LENGTH, stdin) == NULL) {
            printf("ERROR input\n");
        }
	*/
	
        char *pch = strtok(line_in," ");

	
	States state = FIND_INSTR;
        while (pch != NULL) 
        {
		switch (state) {
			case FIND_INSTR:
				if (strcmp(pch, "print") == 0)
				{
					state = FIND_ARG;
					break;
				}
				else
				{
					printf("Syntax Error %s\n", line_in);
					exit(1);
				}
			break;

			case FIND_ARG:
				printf("%s ", pch);
			break;

			
		}

            	// printf(">%s\n",pch);
            	pch = strtok (NULL, " ");
        
    	}
	
	//print here \n
        

    }

}
