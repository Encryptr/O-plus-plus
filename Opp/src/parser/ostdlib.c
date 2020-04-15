#include "ostdlib.h"

void echo(struct Opp_List* args)
{
	for (int i=0; i<args->size; i++)
	{
		struct Opp_Obj* res = opp_eval_expr(args->list[i]);
		
		switch (res->obj_type)
		{
			case OBJ_INT:
				printf("%d ", res->oint);
				break;

			case OBJ_FLOAT:
				printf("%.1lf ", res->ofloat);
				break;

			case OBJ_STR:
				printf("%s ", res->ostr);
				break;

			case OBJ_BOOL:
				printf("%d ", res->obool);
				break;
		}
	}
	printf("\n");
}