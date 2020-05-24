#include "enviroment.h"

unsigned int hash_str(char *string, struct Table* t)
{
	unsigned int h = 0;
	while (*string) {
		h = ((h<<5)-h)^(unsigned char)*(string++);
	}
	h = h % t->size;
	return h;
}

struct Table* createMap(unsigned int size)
{
	struct Table* t = (struct Table*)malloc(sizeof(struct Table));

	if (t == NULL)
		internal_error("MALLOC FAIL", 1);

	t->list = (struct Hash_Node**)malloc(sizeof(struct Hash_Node*)*size);
	if (t->list == NULL)
		internal_error("MALLOC FAIL", 1);

	t->size = size;
	
	for (int i=0;i<size;++i) {
		t->list[i] = NULL;
	}
	return t;
}

void opp_init_environment()
{
	global_ns = init_namespace("Global", NULL);
	current_ns = global_ns;
}

struct Namespace* init_namespace(char* name, struct Namespace* parent)
{
	struct Namespace* ns = (struct Namespace*)malloc(sizeof(struct Namespace));

	if (ns == NULL) 
		internal_error("MALLOC FAIL", 1);

	ns->name = name;
	ns->parent = parent;
	ns->inside = createMap(__HASH_SIZE__); 

	return ns;
}

void delete_node(struct Table* t, char* key)
{
	unsigned int loc = hash_str(key, t);

	if (t->list[loc] == NULL)
		return;
	free(t->list[loc]);
}

int env_get_type(struct Table *t, char *key)
{
	unsigned int loc = hash_str(key, t);
	struct Hash_Node* pos = t->list[loc];

	if (t->list[loc] == NULL)
		return -1;
	else {
		while (pos) {
			if (!strcmp(pos->key, key)) {
				return pos->type;
			}
			if (pos->next != NULL)
				pos = pos->next;
			else 
				return -1;
		}
	}
	return -1;
}

int env_get_int(struct Table *t, char* key)
{
	unsigned int loc = hash_str(key, t);
	struct Hash_Node* pos = t->list[loc];

	if (t->list[loc] == NULL)
		return -1;
	else {
		while (pos) {
			if (!strcmp(pos->key, key)) {
				if (pos->type == VINT)
					return pos->value.ival;
				else return -1;
			}
			if (pos->next != NULL)
				pos = pos->next;
			else 
				return -1; 
		}
	}
	return -1;
}

double env_get_dbl(struct Table *t, char* key)
{
	unsigned int loc = hash_str(key, t);
	struct Hash_Node* pos = t->list[loc];

	if (t->list[loc] == NULL)
		return -1.0;
	else {
		while (pos) {
			if (!strcmp(pos->key, key)) {
				if (pos->type == VDOUBLE)
					return pos->value.dval;
				else return -1.0;
			}
			if (pos->next != NULL)
				pos = pos->next;
			else 
				return -1.0; 
		}
	}
	return -1.0;
}

char* env_get_str(struct Table *t, char* key)
{
	unsigned int loc = hash_str(key, t);
	struct Hash_Node* pos = t->list[loc];

	if (t->list[loc] == NULL)
		return NULL;
	else {
		while (pos) {
			if (!strcmp(pos->key, key)) {
				if (pos->type == VSTR)
					return pos->value.strval;
				else return NULL;
			}
			if (pos->next != NULL)
				pos = pos->next;
			else 
				return NULL; 
		}
	}
	return NULL;
}

void* env_get_cfn(struct Table *t, char* key)
{
	unsigned int loc = hash_str(key, t);
	struct Hash_Node* pos = t->list[loc];

	if (t->list[loc] == NULL)
		return NULL;
	else {
		while (pos) {
			if (!strcmp(pos->key, key)) {
				if (pos->type == VCFUNC)
					return pos->func->cfn;
				else return NULL;
			}
			if (pos->next != NULL)
				pos = pos->next;
			else 
				return NULL; 
		}
	}
	return NULL;
}

struct Hash_Node* env_get_fn(struct Table *t, char* key)
{
	unsigned int loc = hash_str(key, t);
	struct Hash_Node* pos = t->list[loc];

	if (t->list[loc] == NULL)
		return NULL;
	else {
		while (pos) {
			if (!strcmp(pos->key, key)) {
				if (pos->type == VFUNC)
					return pos;
				else return NULL;
			}
			if (pos->next != NULL)
				pos = pos->next;
			else 
				return NULL; 
		}
	}
	return NULL;
}

bool env_new_cfn(struct Table *t, char* key, struct Opp_Obj* (*fn)(struct Opp_List* args))
{
	unsigned int loc = hash_str(key, t);
	struct Hash_Node* pos = t->list[loc];
	struct Hash_Node* new_node = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));

	if (t->list[loc] != NULL) {
		while (pos) {
			if (!strcmp(pos->key, key)) {
				pos->func = (struct Opp_Func*)malloc(sizeof(struct Opp_Func));
				pos->func->cfn = fn;
				return true;
			}
			if (pos->next == NULL) {
				pos->next = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));
				pos->next->type = VCFUNC;
				strcpy(pos->next->key, key);
				pos->next->func = (struct Opp_Func*)malloc(sizeof(struct Opp_Func));
				pos->next->func->cfn = fn;
				return true;
			}
			else
				pos = pos->next;
		}
	}
	else {
		new_node->type = VCFUNC;
		strcpy(new_node->key, key);
		new_node->func = (struct Opp_Func*)malloc(sizeof(struct Opp_Func));
		new_node->func->cfn = fn;
		t->list[loc] = new_node;
	}

	return true;
}

bool env_new_str(struct Table *t, char* key, char* value)
{
	unsigned int loc = hash_str(key, t);
	int len = strlen(value)+1;
	struct Hash_Node* pos = t->list[loc];
	struct Hash_Node* new_node = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));

	if (t->list[loc] != NULL) {
		while (pos) {
			if (!strcmp(pos->key, key)) {
				free(pos->value.strval); // TEMP SOLUTION
				pos->value.strval = (char*)malloc(strlen(value)+1);
				strcpy(pos->value.strval, value);
				return true;
			}
			if (pos->next == NULL) {
				pos->next = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));
				pos->next->type = VSTR;
				strcpy(pos->next->key, key);
				pos->next->value.strval = malloc(sizeof(char)*len);
				strcpy(pos->next->value.strval, value);
				return true;
			}
			else
				pos = pos->next;
		}
	}
	else {
		new_node->type = VSTR;
		strcpy(new_node->key, key);
		new_node->value.strval = malloc(sizeof(char)*len);
		strcpy(new_node->value.strval, value);
		t->list[loc] = new_node;
	}

	return true;
}

bool env_new_int(struct Table *t, char* key, int value)
{
	unsigned int loc = hash_str(key, t);
	struct Hash_Node* pos = t->list[loc];
	struct Hash_Node* new_node = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));

	if (t->list[loc] != NULL) {
		while (pos) {
			if (!strcmp(pos->key, key)) {
				pos->value.ival = value;
				return true;
			}
			if (pos->next == NULL) {
				pos->next = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));
				pos->next->type = VINT;
				strcpy(pos->next->key, key);
				pos->next->value.ival = value;
				return true;
			}
			else
				pos = pos->next;
		}
	}
	else {
		new_node->type = VINT;
		strcpy(new_node->key, key);
		new_node->value.ival = value;
		t->list[loc] = new_node;
	}

	return true;
}

bool env_new_dbl(struct Table *t, char* key, double value)
{
	unsigned int loc = hash_str(key, t);
	struct Hash_Node* pos = t->list[loc];
	struct Hash_Node* new_node = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));

	if (t->list[loc] != NULL) {
		while (pos) {
			if (!strcmp(pos->key, key)) {
				pos->value.dval = value;
				return true;
			}
			if (pos->next == NULL) {
				pos->next = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));
				pos->next->type = VDOUBLE;
				strcpy(pos->next->key, key);
				pos->next->value.dval = value;
				return true;
			}
			else
				pos = pos->next;
		}
	}
	else {
		new_node->type = VDOUBLE;
		strcpy(new_node->key, key);
		new_node->value.dval = value;
		t->list[loc] = new_node;
	}

	return true;
}

bool env_new_bool(struct Table *t, char* key, int value)
{
	unsigned int loc = hash_str(key, t);
	struct Hash_Node* pos = t->list[loc];
	struct Hash_Node* new_node = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));

	if (t->list[loc] != NULL) {
		while (pos) {
			if (!strcmp(pos->key, key)) {
				pos->value.bval = value;
				return true;
			}
			if (pos->next == NULL) {
				pos->next = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));
				pos->next->type = VBOOL;
				strcpy(pos->next->key, key);
				pos->next->value.bval = value;
				return true;
			}
			else
				pos = pos->next;
		}
	}
	else {
		new_node->type = VBOOL;
		strcpy(new_node->key, key);
		new_node->value.bval = value;
		t->list[loc] = new_node;
	}

	return true;
}

bool env_new_fn(struct Table *t, char* key, struct Opp_Stmt* stmts, struct Opp_List* args)
{
	unsigned int loc = hash_str(key, t);
	struct Hash_Node* pos = t->list[loc];
	struct Hash_Node* new_node = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));

	if (t->list[loc] != NULL) {
		while (pos) {
			if (!strcmp(pos->key, key)) {
				pos->func = (struct Opp_Func*)malloc(sizeof(struct Opp_Func));
				pos->func->stmts = stmts;
				pos->func->arg_name = args;
				return true;
			}
			if (pos->next == NULL) {
				pos->next = (struct Hash_Node*)malloc(sizeof(struct Hash_Node));
				pos->next->type = VFUNC;
				strcpy(pos->next->key, key);
				pos->next->func = (struct Opp_Func*)malloc(sizeof(struct Opp_Func));
				pos->next->func->stmts = stmts;
				pos->next->func->arg_name = args;
				return true;
			}
			else
				pos = pos->next;
		}
	}
	else {
		new_node->type = VFUNC;
		strcpy(new_node->key, key);
		new_node->func = (struct Opp_Func*)malloc(sizeof(struct Opp_Func));
		new_node->func->stmts = stmts;
		new_node->func->arg_name = args;
		t->list[loc] = new_node;
	}

	return true;
}

void env_add_local(struct Table* t, char* key, struct Opp_List* args, struct Opp_List* name)
{
	if (args->size != name->size)
		opp_error(NULL, "Invalid amount of args provided to function '%s'", key);

	for (int i = 0; i < args->size; i++) {

		struct Opp_Obj* res = opp_eval_expr(args->list[i]);
		struct Opp_Expr* a = (struct Opp_Expr*)(name->list[i]);
		struct Opp_Expr_Unary* id = (struct Opp_Expr_Unary*)(a->expr);

		switch (res->obj_type)
		{
			case OBJ_INT: 
				env_new_int(t, id->val.strval, res->oint);
				break;

			case OBJ_FLOAT:
				env_new_dbl(t, id->val.strval, res->ofloat);
				break;

			case OBJ_STR:
				env_new_str(t, id->val.strval, res->ostr);
				break;

			case OBJ_BOOL:
				env_new_bool(t, id->val.strval, res->obool);
				break;
		}
	}
}