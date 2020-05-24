#ifndef OLISP_ENV
#define OLISP_ENV

/*   CHANGE IF NEEDED   */
#define __HASH_SIZE__ 1000
/************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../util/error.h"
#include "../parser/ast.h"
#include "../lexer/lexer.h"
#include "../interpreter/interpreter.h"

struct Namespace;

struct Opp_Func {
	int line;
	struct Opp_Obj* (*cfn)(struct Opp_List* args);
	struct Opp_Value ret_val;
	struct Opp_Stmt* stmts;
	struct Opp_List* arg_name;
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
bool env_new_bool(struct Table *t, char* key, int value);
bool env_new_dbl(struct Table *t, char* key, double value);
bool env_new_cfn(struct Table *t, char* key, struct Opp_Obj* (*fn)(struct Opp_List* args));

bool env_new_fn(struct Table *t, char* key, struct Opp_Stmt* stmts, struct Opp_List* args);
bool env_lookup(struct Table* t, char* key);

int env_get_type(struct Table *t, char *key);
int env_get_int(struct Table *t, char* key);
double env_get_dbl(struct Table *t, char* key);
char* env_get_str(struct Table *t, char* key);
void* env_get_cfn(struct Table *t, char* key);
struct Hash_Node* env_get_fn(struct Table *t, char* key);

void env_add_local(struct Table* t, char* key, struct Opp_List* args, struct Opp_List* name);

#endif