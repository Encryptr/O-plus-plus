#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../hashmap.c"

char* words[] = {
	"test", "hello", "var",
	"int", "i", "a", "x", "name"
};


int main()
{
	struct Table* t = createMap();
	unsigned int location = 0;

	insert_str(t, words[0], "STRING::4");
	insert_int(t, words[1], 65);
	insert_float(t, words[2], -2.3);
	insert_str(t, words[3], "INT");

	for (int i=0;i<sizeof(words)/sizeof(words[0]); ++i)
	{
		location = hash_str(words[i]);

		if (t->list[location] != NULL)
		{
			switch (t->list[location]->type)
			{
				case STRING:
					printf("[%s]\n", t->list[location]->v3);
				break;

				case INT:
					printf("[%d]\n", t->list[location]->v1);
				break;

				case FLOAT:
					printf("[%lf]\n", t->list[location]->v2);
				break;

			}
		}
		else {
			printf("==>%s NOT INSERTED\n", words[i]);
		}

	}
}
