#include "type.c"

#define TEST 100

struct Obj {
	enum Token type;
	int num;
	char string[TEST];
	bool op;
	struct Obj *car, *cdr;
};

struct Hash_Node {
	enum Types type;

	union {
		int v1;
		double v2;
		char* v3;
	};
	char key[20];
	struct Hash_Node *next;
};

struct Table {
	int size;
	struct Hash_Node **list;
};

struct Obj* analize(struct Scan* d);
struct Obj* make(int type, int num);

struct Obj* binary_cond(struct Scan *d, int type);
struct Obj* con(struct Scan *d);
struct Obj* number(struct Scan *d);
struct Obj* string(struct Scan *d);
struct Obj* list_make(struct Scan *d);
struct Obj* import(struct Scan *d);

struct Table* createMap(int size);
uint32_t hash_str(const char * data, int len);
void insert(struct Table *t, int hash);
int lookup(struct Table* t, int hash);
