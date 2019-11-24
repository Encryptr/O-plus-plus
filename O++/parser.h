#define TEST 100

struct Obj {
	enum Token type;
	int num;
	char string[TEST];
	bool op;
	struct Obj *car, *cdr;
};

struct Obj* analize(struct Scan* d);
struct Obj* make(int type, int num);

struct Obj* binary_cond(struct Scan *d, int type);
struct Obj* con(struct Scan *d);
struct Obj* number(struct Scan *d);
struct Obj* string(struct Scan *d);
struct Obj* list_make(struct Scan *d);
struct Obj* import(struct Scan *d);
