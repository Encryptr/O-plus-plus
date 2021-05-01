#include "types.h"
#include "../memory/memory.h"
#include "../util/util.h"

struct Opp_Type* opp_create_type(enum Opp_Type_T type, struct Opp_Type* prev)
{
	struct Opp_Type* t = (struct Opp_Type*)
		opp_alloc(sizeof(struct Opp_Type));

	if (!t)
		MALLOC_FAIL();

	memset(t, 0, sizeof(struct Opp_Type));

	t->type = type;
	t->next = prev;

	return t;
}

void opp_debug_type(struct Opp_Type* type)
{
	static int depth = 0;
	depth++;

	if (type->storage_class != 0)
		printf("%s ", tok_debug(type->storage_class));
	if (type->const_attr)
		printf("const ");
	if (type->volatile_attr)
		printf("volatile ");
	if (type->unsign)
		printf("unsigned ");

	switch (type->type)
	{
		case TYPE_VOID:
			printf("void ");
			break;

		case TYPE_CHAR:
			printf("char ");
			break;

		case TYPE_SHORT:
			printf("short ");
			break;

		case TYPE_INT:
			printf("int ");
			break;

		case TYPE_LONG:
			printf("long ");
			break;

		case TYPE_FLOAT:
			printf("float ");
			break;

		case TYPE_DOUBLE:
			printf("double ");
			break;

		case TYPE_LDOUBLE:
			printf("long double ");
			break;

		case TYPE_PTR: {
			opp_debug_type(type->next);

			printf("* ");
			if (type->const_ptr)
				printf("const ");
			if (type->restrict_ptr)
				printf("restrict ");
			if (type->volatile_ptr)
				printf("volatile ");
			break;
		}

		case TYPE_FUNC: {
			opp_debug_type(type->next);
			printf("( ");
			for (unsigned int i = 0; i < type->val.fn.len; i++) {
				opp_debug_type(type->val.fn.param[i].type);
				if ((i+1) < type->val.fn.len)
					printf(", ");
			}
			printf(")");
			break;
		}

		default:
			break;
	}

	depth--;
	if (depth == 0)
		printf("\n");
}

/*
	Types are parsed with regular precedence rules. 
	Exeption is when a nested () expression is exited.
	In that case we need to append to the current type to the
	end of the resulting nested type.

	int (*(*lol)())()
	
	Ptr -> func -> ptr -> func -> int
	
	int (*a)()
	
	Ptr -> func -> int

	int * a ()

	Func -> ptr -> int
*/

struct Opp_Type* opp_type_fix(struct Opp_Type* inner, struct Opp_Type* outer)
{
	struct Opp_Type* t = inner;

	while ((t->type == TYPE_PTR || t->type == TYPE_FUNC) 
			&& t->next != NULL) 
	{
		t = t->next;
	}

	t->next = outer;

	return inner;
}
