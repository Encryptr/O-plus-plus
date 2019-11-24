#include "hashmap.h"

unsigned long hash_str(char *string)
{
    unsigned long hash = 5381;
    int c;

    while (c = *string++)
        hash = ((hash << 5) + hash) + c; 

    return hash;
}

struct Table* createMap(unsigned long size)
{
	struct Table* t = (struct Table*)malloc(sizeof(struct Table));
	t->list = (struct Hash_Node**)malloc(sizeof(struct Hash_Node*)*size);
	for (int i=0;i<size;++i) {
		t->list[i] = (struct Table*)malloc(size*sizeof(struct Table));
	}
	return t;
}
