
//--------------------------------------
struct Obj* eval(struct Obj* node);
struct Obj* eval_import(struct Obj* node);

struct Obj* plus(struct Obj* node);
struct Obj* minus(struct Obj* node);
struct Obj* times(struct Obj* node);
struct Obj* divide(struct Obj* node);
struct Obj* stmt_eval(struct Obj* node);
struct Obj* if_stmt(struct Obj* node);

struct Obj* print(struct Obj* node);
//--------------------------------------

struct Obj* eval(struct Obj* node)
{
	switch (node->type)
	{
		case CON:
			return eval(node->car);
		break;

		case TIMPORT:
			return eval_import(node);
		break;

		case PLUS:  return plus(node->cdr);  break;
		case MINUS: return minus(node->cdr); break;
		case TIMES: return times(node->cdr); break;
		case DIVIDE:return divide(node->cdr); break;

		case EQ: case LESSTHAN: case MORETHAN:
			return stmt_eval(node);
		break;

		case TPRINT:
			return print(node->cdr);
		break;

		case TIF:
			return if_stmt(node);
		break;

		case NUM: case NIL: case TLIST: 
		case TTRUE: case TFALSE: case IDENT:
			return node;
		break;
	}
	printf("Invalid node type...\n");
	printf("ERROR: %d\n", node->type);
	exit(1);
	//return eval(node->cdr);
}

struct Obj* eval_import(struct Obj* node)
{
	struct Obj* obj;
	struct Obj* fileobj;
	
	init_opp(node->string, fileobj);

	obj = make(NUM, 0);

	return obj;
}

struct Obj* if_stmt(struct Obj* node)
{
	struct Obj* stmt;
	stmt = eval(node->cdr);

	if (stmt->type == TTRUE)
		return eval(node->cdr->cdr);
	else if (stmt->type == TFALSE)
	{
		if (node->cdr->cdr->cdr == NULL || 
			node->cdr->cdr->cdr->type == NIL)
			PRINT_ERROR(3);
		return eval(node->cdr->cdr->cdr);
	}
	else 
		PRINT_ERROR(1);
}

struct Obj* stmt_eval(struct Obj* node)
{
	struct Obj* next;
	bool result = false;
	int n1 = 0, n2, operand = node->type;
	
	next = eval(node->cdr);
	n1 = next->num;

	next = eval(node->cdr->cdr);
	n2 = next->num;

	switch (operand)
	{
		case EQ:
		{
			if (n1 == n2)
			{next = make(TTRUE, 1);break;}
			else {next = make(TFALSE, 0);break;}
		}
		case MORETHAN:
		{
			if (n1 > n2)
				{next = make(TTRUE, 1); break;}
			else {next = make(TFALSE,0); break;}
		}
		case LESSTHAN:
		{
			if (n1 < n2)
				{next = make(TTRUE, 1); break;}
			else {next = make(TFALSE,0); break;}
		}
		default:
			printf("Error in operand\n"); exit(1);
		break;
	}

	return next;
}

struct Obj* plus(struct Obj* node)
{
	int result = node->num;
	struct Obj* next;

	for (next = node->cdr;next->type!=NIL;next=next->cdr)
	{
		struct Obj* p = eval(next);
		if (p->type == NUM)
			result += p->num;
	}
	next = make(NUM, result);

	return next;
}

struct Obj* minus(struct Obj* node)
{
	int result = node->num;
	struct Obj* next;

	for (next = node->cdr;next->type!=NIL;next=next->cdr)
	{
		struct Obj* p = eval(next);
		if (p->type == NUM)
			result -= p->num;
	}
	next = make(NUM, result);

	return next;
}

struct Obj* times(struct Obj* node)
{
	int result = node->num;
	struct Obj* next;

	for (next = node->cdr;next->type!=NIL;next=next->cdr)
	{
		struct Obj* p = eval(next);
		if (p->type == NUM)
			result *= p->num;
	}
	next = make(NUM, result);

	return next;
}

struct Obj* divide(struct Obj* node)
{
	int result = node->num;
	struct Obj* next;

	for (next = node->cdr;next->type!=NIL;next=next->cdr)
	{
		struct Obj* p = eval(next);
		if (p->type == NUM)
			result /= p->num;
	}
	next = make(NUM, result);

	return next;
}

struct Obj* print(struct Obj* node)
{
	switch (node->type)
	{
		case NUM: printf("%d\n", node->num); break;
		case CON: print(eval(node)); break;
		case NIL: printf("NIL\n"); break;
		case IDENT: printf("%s\n", node->string); break;
		case TLIST: 
		{
			printf("(");
			struct Obj* next;
			for (next = node->cdr;next->type!=NIL;next=next->cdr)
			{
				struct Obj* p = eval(next);
				// ADD MULTI LIST
				if (p->type == NUM)
					printf("%d ", p->num);
			}
			printf("\b)\n");
			break;
		}
		case TTRUE: printf("true\n"); break;
		case TFALSE: printf("false\n"); break;

		default:
			PRINT_ERROR(4);
		break;
	}
}