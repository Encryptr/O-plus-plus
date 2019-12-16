#include "ast.h"

struct Opp_Node* opp_make(enum Ast_Type type, struct Opp_Value value)
{
	struct Opp_Node* node;

	node = (struct Opp_Node*)malloc(sizeof(struct Opp_Node));

	node->type = type;
	node->rt = NULL;
	node->lt = NULL;
	node->value = value;

	return node;
}

struct Opp_Node* opp_new_node(enum Ast_Type type, struct Opp_Node* op1, 
							struct Opp_Node* op2, struct Opp_Value value)
{
	struct Opp_Node* node;

	node = (struct Opp_Node*)malloc(sizeof(struct Opp_Node));

	node->type = type;
	node->rt = op1;
	node->lt = op2;
	node->value = value;

	return node;	
}

struct Opp_Value null_opp_value()
{
	struct Opp_Value val = {0};

	return val;
}

/*

void opp_debug_tree(struct Opp_Ast* oppTree)
{
	struct Opp_Node* cur = oppTree->tree;

	if (cur == NULL)
		printf("DEBUG ERROR NULL\n"), exit(1);

	if (cur->type == OPP_AST_START)
		cur = oppTree->tree->rt;

	while (cur->type != AST_END)
	{
		switch (cur->type)
		{
			case FEND:

			break;

			case AST_INTEGER:

			break;
		}
		// if (cur->rt == NULL)
		// {
		// 	if (cur->lt == NULL)
		// 		return;
		// }
	}
}

*/