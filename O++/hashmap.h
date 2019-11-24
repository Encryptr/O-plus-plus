enum Types {
	INT, CONST_INT, CONST_FLOAT, 
	FLOAT, STRING
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

struct Table* createMap(unsigned long size);
unsigned long hash_str(char *string);
void insert(struct Table *t, int hash);
int lookup(struct Table* t, int hash);