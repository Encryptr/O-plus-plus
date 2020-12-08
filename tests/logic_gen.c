#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include ".././opp/opp.h"

static const char* cmp[] = {
	[TEQEQ] = "je", [TNOTEQ] = "jne", [TLT] = "jl", 
	[TGT] = "jg", [TLE] = "jle", [TGE] = "jge"
};

static const char* sym[] = {
	[TEQEQ] = "==", [TNOTEQ] = "!=", [TLT] = "<", 
	[TGT] = ">", [TLE] = "<=", [TGE] = ">="
};

int label_loc;

#define TRU 0
#define FAL 1
#define AND 1
#define OR 2

#define RIGHT 1
#define LEFT 2

struct Opts {
	int curr, use_op, jloc, 
		hs,
		jmps[2];
} opts;

static int ops[] = {
	[TGT] = TLE,
	[TLT] = TGE,
	[TEQEQ] = TNOTEQ,
	[TGE] = TLT,
	[TLE] = TGT,
	[TNOTEQ] = TEQEQ
};

void analize(struct Opp_Node* node) 
{
	struct Opts save = opts;
	switch (node->logic_expr.tok) 
	{
		case TAND: {
			opts.curr = AND;
			
			if (node->logic_expr.left->logic_expr.tok == TOR)
				opts.jmps[TRU] = label_loc++;

			opts.use_op = 1;
			opts.hs = LEFT;
			opts.jloc = opts.jmps[FAL];
			analize(node->logic_expr.left);

			if (node->logic_expr.left->logic_expr.tok == TOR) {
				printf(".L%d\n", opts.jmps[TRU]);
				opts.jmps[TRU] = save.jmps[TRU];
			}

			if (save.curr == OR && save.hs == LEFT) {
				opts.use_op = 0;
				opts.jloc = opts.jmps[TRU];
			}
			else if (save.hs == RIGHT && save.curr == OR) {
				opts.use_op = 0;
				opts.jloc = opts.jmps[TRU];
			}
			else {
				opts.use_op = 1;
				opts.jloc = opts.jmps[FAL];
			}
			opts.hs = RIGHT;
			opts.curr = save.curr;
			analize(node->logic_expr.right);

			break;
		}

		case TOR: {
			opts.curr = OR;

			if (node->logic_expr.left->logic_expr.tok == TAND)
				opts.jmps[FAL] = label_loc++;

			opts.use_op = 0;
			opts.jloc = opts.jmps[TRU];
			opts.hs = LEFT;
			analize(node->logic_expr.left);

			if (node->logic_expr.left->logic_expr.tok == TAND) {
				printf(".L%d\n", opts.jmps[FAL]);
				opts.jmps[FAL] = save.jmps[FAL];
			}

			if (save.curr == OR && save.hs == LEFT) {
				opts.use_op = 0;
				opts.jloc = opts.jmps[TRU];
			}
			else if (save.hs == RIGHT && save.curr == OR) {
				opts.use_op = 0;
				opts.jloc = opts.jmps[TRU];
			}
			else {
				opts.use_op = 1;
				opts.jloc = opts.jmps[FAL];
			}
			opts.hs = RIGHT;
			opts.curr = save.curr;
			analize(node->logic_expr.right);
			
			break;
		}

		default: {
			printf("\tcmp %lld, %lld\n", 
				node->logic_expr.left->unary_expr.val.i64val,
				node->logic_expr.right->unary_expr.val.i64val);

			if (opts.use_op)
				printf("\t%s .L%d\n", cmp[ops[node->logic_expr.tok]], opts.jloc);
			else
				printf("\t%s .L%d\n", cmp[node->logic_expr.tok], opts.jloc);
			break;
		}
	}
	opts = save;
}
// gcc ../opp/lexer/lexer.c ../opp/parser/parser.c logic_gen.c 

// BASIC TEST PASSING
// char* src = "main() (1 == 1 || 2 == 2) || ( 3 == 3 || 4 == 4);";
// char* src = "main() (1 > 1 || 2 < 2) && 3 == 3;";
// char* src = "main() (1 != 1);";


// RIGHT SIDE OUTER TESTS PASSING
// char* src = "main() ((1 == 1 || 2 == 2) && ( 3 == 3 && 4 == 4) ) || (5 == 5 && 6 == 6) && (7 == 7 && 8 == 8);"; // WORKS ***
// char* src = "main() ((1 == 1 || 2 == 2) && ( 3 == 3 || 4 == 4) ) || (5 == 5 && 6 == 6) && (7 == 7 && 8 == 8);"; // WORKS ***


// char* src = "main() (1 == 1 || 2 == 2) || ( 3 == 3 || 4 == 4);";
// char* src = "main() (1 == 1 && 2 == 2) || (3 == 3 || 4 == 4);";


int main() {
	struct Opp_Scan scan = {0};
	init_opp_lex(&scan, src);
	struct Opp_Parser* parser = opp_parser_init(&scan);
	opp_parser_begin(parser);

	// Setup
	opts.jmps[TRU] = label_loc++;
	opts.jmps[FAL] = label_loc++;
	opts.jloc = opts.jmps[FAL];
	opts.use_op = 1;
	opts.curr = 0;

	analize(parser->statments[0]->fn_stmt.body);

	printf(".L0:\n\tmov eax, 1\n\tjmp .L2\n");
	printf(".L1:\n\tmov eax, 0\n.L2: ...\n");

	return 0;
}