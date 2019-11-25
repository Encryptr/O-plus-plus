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

// REVIEW FOR DEPRICATION
// struct Hash_Node* new_node(const char* key)
// {
// 	struct Hash_Node* node = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));
// 	strcpy(node->key, key);
// 	node->next = NULL;

// 	return node;
// }

struct Table* createMap()
{
	struct Table* t = (struct Table*)malloc(sizeof(struct Table));

	t->list = (struct Hash_Node**)malloc(sizeof(struct Hash_Node*)*__HASH_SIZE__);
	
	for (int i=0;i<__HASH_SIZE__;++i) {
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
