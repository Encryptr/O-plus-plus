#include "hashmap.h"

unsigned long hash_str(char *string)
{
	unsigned long h = 0;
	while (*string) {
		h = ((h<<5)-h)^(unsigned char)*(string++);
	}
	h = h % __HASH_SIZE__;
	return h;
}

struct Table* createMap(int size)
{
	struct Table* t = (struct Table*)malloc(sizeof(struct Table));

	t->list = (struct Hash_Node**)malloc(sizeof(struct Hash_Node*)*size);
	
	for (int i=0;i<size;++i) {
		t->list[i] = NULL;
	}
	return t;
}

void free_table(struct Table* t)
{
	free(t);
	free(t->list);
}

void delete_node(struct Table* t, char* key)
{
	unsigned int loc = hash_str(key);

	if (t->list[loc] == NULL)
		return;
	free(t->list[loc]);
}

int insert_str(struct Table *t, char* key, char* value)
{
	unsigned int loc = hash_str(key);
	int len = strlen(value)+1;

	if (t->list[loc] != NULL)
		return 0;

	t->list[loc] = malloc(sizeof(struct Hash_Node));

	// INIT
	t->list[loc]->type = STRING;
	strcpy(t->list[loc]->key, key);
	t->list[loc]->v3 = malloc(sizeof(char)*len);
	strcpy(t->list[loc]->v3, value);

	// IMPLEMENT DUPLICATES
	return 1;
}

int insert_int(struct Table *t, char* key, int value)
{
	unsigned int loc = hash_str(key);

	if (t->list[loc] != NULL)
		return 0;

	t->list[loc] = malloc(sizeof(struct Hash_Node));

	// INIT
	t->list[loc]->type = INT;
	strcpy(t->list[loc]->key, key);
	t->list[loc]->v1 = value;

	// IMPLEMENT DUPLICATES
	return 1;
}

int insert_float(struct Table *t, char* key, double value)
{
	unsigned int loc = hash_str(key);

	if (t->list[loc] != NULL)
		return 0;

	t->list[loc] = malloc(sizeof(struct Hash_Node));

	// INIT
	t->list[loc]->type = FLOAT;
	strcpy(t->list[loc]->key, key);
	t->list[loc]->v2 = value;

	// IMPLEMENT DUPLICATES
	return 1;
}

int insert_Cfunc(struct Table *t, char* key, void (*fn)(struct Scan* s, struct Table* local))
{
	unsigned int loc = hash_str(key);

	if (t->list[loc] != NULL)
		return 0;

	t->list[loc] = malloc(sizeof(struct Hash_Node));

	// INIT
	t->list[loc]->type = CFUNC;
	t->list[loc]->func.cfn = fn;

	return 1;
}

int insert_func(struct Table *t, unsigned int element, char* key)
{
	if (t->list[element] != NULL)
		return 0;

	t->list[element] = malloc(sizeof(struct Hash_Node));
	t->list[element]->type = FUNC;
	strcpy(t->list[element]->key, key);

	return 1;
}

enum Types check_type(struct Table *t, char *key)
{
	unsigned int loc = hash_str(key);

	if (t->list[loc] == NULL)
		return ERROR;
	return t->list[loc]->type;
}
