#include <stdio.h>
#include <time.h>
#include "enviroment.h"

// Test OLisp enviroment

/* Collision example
Two same hashing str:
ytqzgysahb
czpcncziei
ajsuqhwvkp

The Env should avoid a collison

*/

void collison_test() 
{ 
	/*
	// FINDING ONE OF THESE STRING
	char in1[11] = {0};
	char in2[11] = {0};
	char in3[11] = {0};

	while (1) {
		for (int i=0; i<10; i++) {
			// int a1 = (rand() % (122 - 97 + 1)) + 97;
			// int a2 = (rand() % (122 - 97 + 1)) + 97;  
			int a3 = (rand() % (122 - 97 + 1)) + 97;
			// in1[i] = a1;
			// in2[i] = a2;
			in3[i] = a3;
		}
		if ((hash_str("ytqzgysahb", global_ns->inside)) == (hash_str("czpcncziei", global_ns->inside)) && (hash_str("czpcncziei", global_ns->inside)) == (hash_str(in3, global_ns->inside)))
		{
			printf("Match: [%s] = [%s] = [%s]\n", in1, in2, in3);
			exit(1);
		}
	}
	*/
	
	unsigned int h1 = hash_str("ytqzgysahb", global_ns->inside);
	unsigned int h2 = hash_str("czpcncziei", global_ns->inside);
	unsigned int h3 = hash_str("ajsuqhwvkp", global_ns->inside);
	printf("%u %u %u\n", h1, h2, h3);

	env_new_str(global_ns->inside, "ytqzgysahb", "hello");
	env_new_int(global_ns->inside, "czpcncziei", 123);
	env_new_dbl(global_ns->inside, "ajsuqhwvkp", 99.23);

	printf("%s\n", global_ns->inside->list[h1]->value.strval);
	printf("%d\n", global_ns->inside->list[h1]->next->value.ival);
	printf("%lf\n", global_ns->inside->list[h1]->next->next->value.dval);

	printf("Type: %d\n", env_get_type(global_ns->inside, "ajsuqhwvkp"));

	printf("Val: %d\n", env_get_int(global_ns->inside, "czpcncziei"));
}

int main() 
{
	srand(time(NULL));
	olisp_init_environment();

	// Global var
	global_ns->inside->list[1] = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));
	global_ns->inside->list[1]->val = 23;

	// Function
	global_ns->inside->list[0] = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));
	global_ns->inside->list[0]->func = malloc(sizeof(struct OLisp_Func));
	global_ns->inside->list[0]->func->local = init_namespace("func1", global_ns);
	global_ns->inside->list[0]->func->local->inside->list[1] = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));
	global_ns->inside->list[0]->func->local->inside->list[1]->val = 99;
	global_ns->inside->list[0]->func->local->parent = global_ns;

	current_ns = global_ns->inside->list[0]->func->local;

	// Search for value
	struct Namespace* search = current_ns;
	while (search != NULL)
	{
		if (search->inside->list[1] != NULL) {
			// Finds local 99 first then 23 
			printf("===>%d\n", search->inside->list[1]->val);
		}
		search = search->parent;
	}
	free(global_ns->inside->list[0]->func->local->inside->list[1]);
	free(global_ns->inside->list[0]->func->local->inside->list);

	collison_test();

	return 0;
}