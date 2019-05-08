#include "o++.h"
#include "parser.c"

Class class = FCLASS;


void check_main(char intake[1000])
{
	str_delim = str_delim_def;
    char *sword = strtok(fline, str_delim);

    while (sword != NULL)
    {
    	switch (class)
    	{
    		case FCLASS:
    			if (strcmp(sword, stdlib[0]) == 0)
    			{
    				has_class = 1;
    				class = CCONT;
    				break;
    			}
    		break;

    		case CCONT:
    			if (strcmp(sword, stdlib[1]) == 0)
         		{
            		lex_class(class_tokens);
            		return;
          		}
          		strcpy(class_tokens[idx], sword);
          		idx++;
    		break;
    	}

    	sword = strtok(NULL, str_delim);	
    }
} 