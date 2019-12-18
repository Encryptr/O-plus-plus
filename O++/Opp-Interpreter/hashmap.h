#ifndef OPP_HASH
#define OPP_HASH

#define __HASH_SIZE__ 10000

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"

enum Types {
	ERROR, INT, CONST_INT, CONST_FLOAT, 
	FLOAT, CFUNC, FUNC, STRING
};

struct Opp_Func {
	char* loc;
	void (*cfn)(struct Scan* s);
	struct Opp_Value ret_val;
	int exp_param;
};

struct Hash_Node {
	enum Types type;
	char key[20];
	union {
		int v1;
		double v2;
		char* v3;
		struct Opp_Func func;
	};
	// struct Hash_Node *next;
};

struct Table {
	int size;
	struct Hash_Node **list;
};

struct Table* map;

unsigned long hash_str(char *string);
struct Table* createMap();
void free_table(struct Table* t);
void delete_node(struct Table* t, char* key);

int insert_str(struct Table *t, char* key, char* value);
int insert_int(struct Table *t, char* key, int value);
int insert_float(struct Table *t, char* key, double value);
int insert_Cfunc(struct Table *t, char* key, void (*fn)(struct Scan* s));
int insert_func(struct Table *t, unsigned int element);


enum Types check_type(struct Table *t, char *key);

int lookup(struct Table* t, int hash);

#endif