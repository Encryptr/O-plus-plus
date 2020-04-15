#ifndef OLISP_ENV
#define OLISP_ENV

#define __HASH_SIZE__ 1000
#define SMALLER_HASH 1000

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../util/error.h"
#include "../parser/ast.h"
#include "../lexer/lexer.h"

struct Namespace;

struct Opp_Func {
	char* loc;
	int line;
	void (*cfn)(struct Opp_Scan* s);
	struct Namespace* local;
};

struct Hash_Node {
	char key[20];
	enum Value_Type type;
	union {
		int val;
		struct Opp_Value value;
		struct Opp_Func* func;
	};
	struct Hash_Node* next;
};

struct Table {
	unsigned int size;
	unsigned int* keys;
	struct Hash_Node **list;
};

struct Namespace {
	const char* name;
	struct Namespace* parent;
	struct Table* inside;
};

// Global table / current namespace
struct Namespace* global_ns;
struct Namespace* current_ns;

unsigned int hash_str(char *string, struct Table* t);
struct Namespace* init_namespace(char* name, struct Namespace* parent);
struct Table* createMap(unsigned int size);
void delete_node(struct Table* t, char* key);
void opp_init_environment();

bool env_new_str(struct Table *t, char* key, char* value);
bool env_new_int(struct Table *t, char* key, int value);
bool env_new_dbl(struct Table *t, char* key, double value);
// bool env_new_cfn(struct Namespace* ns, char* key, void (*fn)(struct OLisp_Scan* s));
// bool env_new_fn(struct Table *t, unsigned int element, char* key);
bool env_lookup(struct Table* t, char* key);
enum Value_Type env_get_type(struct Table *t, char *key);

int env_get_int(struct Table *t, char* key);
double env_get_dbl(struct Table *t, char* key);
char* env_get_str(struct Table *t, char* key);

#endif