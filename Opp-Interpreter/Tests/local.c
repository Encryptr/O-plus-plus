/*

	O++ File for testing local namespaces

	## CURRENTLY WORKING ##

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define HASH_SIZE 200

struct Namespace;
struct Function;
struct Hash_Node;
struct Table;

struct Namespace {
	const char* name;
	struct Namespace* parent;

	// Local hashglobal
	struct Table* inside;
};

struct Namespace* current = NULL;

struct Function {
	const char* name;
	const char* pos;
	struct Namespace* local;
};

enum Types {
	ERROR, INT, CONST_INT, CONST_FLOAT, 
	FLOAT, CFUNC, FUNC, STRING
};

struct Hash_Node {
	enum Types type;
	char key[20];
	union {
		int v1;
		struct Function* func;
	};
};

struct Table {
	struct Hash_Node **list;
};

int main()
{
	// Global table
	struct Namespace* global = (struct Namespace*)malloc(sizeof(struct Namespace));

	global->inside = (struct Table*)malloc(sizeof(struct Table));
	global->inside->list = (struct Hash_Node**)malloc(sizeof(struct Hash_Node*)*HASH_SIZE);
	
	for (int i=0;i<HASH_SIZE;++i) {
		global->inside->list[i] = NULL;
	}

	// Skid over hashing for testing
	global->inside->list[0] = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));
	global->inside->list[0]->type = FUNC;
	strcpy(global->inside->list[0]->key, "test");
	global->inside->list[0]->func = (struct Function*)malloc(sizeof(struct Function));
	global->inside->list[0]->func->name = global->inside->list[0]->key;
	global->inside->list[0]->func->pos = NULL;
	global->inside->list[0]->func->local = (struct Namespace*)malloc(sizeof(struct Namespace));
	global->inside->list[0]->func->local->inside = (struct Table*)malloc(sizeof(struct Table));
	global->inside->list[0]->func->local->parent = global;
	global->inside->list[0]->func->local->inside->list = (struct Hash_Node**)malloc(sizeof(struct Hash_Node*)*HASH_SIZE);
	
	for (int i=0;i<HASH_SIZE;++i) {
		global->inside->list[0]->func->local->inside->list[i] = NULL;
	}

	
	///////// GOOD IN LOCAL /////////
	// Call function test
	current = global->inside->list[0]->func->local;

	global->inside->list[0]->func->local->inside->list[1] = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));
	global->inside->list[0]->func->local->inside->list[1]->type = INT;
	global->inside->list[0]->func->local->inside->list[1]->v1 = 23;

	

	current = global->inside->list[0]->func->local;
	global->inside->list[1] = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));
	global->inside->list[1]->type = INT;
	global->inside->list[1]->v1 = 32;


	// Search for local value

	struct Namespace* search = global->inside->list[0]->func->local;
	while (search != NULL)
	{
		// Hash skiped
		if (search->inside->list[1] != NULL) {
			printf("GOOD\n");
			break;
		}
		search = search->parent;
	}



	return 0;
}

