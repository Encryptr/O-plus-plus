#ifndef PARSER
#define PARSER

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
struct Obj* variable(struct Scan *d);
struct Obj* defvar(struct Scan *d);
struct Obj* setvar(struct Scan *d);
struct Obj* changevar(struct Scan *d);

void warning_dump(struct Scan *d);

void init_file(const char* fname, struct Scan *d, char* source);
void init_opp(const char* fname, struct Obj* root);


#endif