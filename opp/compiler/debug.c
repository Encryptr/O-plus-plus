#include "compiler.h"

/* Structure

Error Example:
	file.o++:line:colum: compile time error: in function "main(...)"
		Invalid unary type in binary expression
		1 + 2 + "hello"
		~~~~~~~~~~~~~~~

Error Example:
	file.o++:line:colum: compile time error: in function "main(...)" near "1"
		Invalid unary type in binary expression
		1 + 2 + "hello"
		~~~~~~~~~~~~~~~
*/

void opp_debug_node(struct Opp_Node* base)
{
	if (base == NULL)
		return;

	switch (base->type)
	{
		case EBIN: {
			opp_debug_node(base->bin_expr.left);
			switch (base->bin_expr.tok)
			{
				case TADD: printf(" + "); break;
				case TMIN: printf(" - "); break;
				case TMUL: printf(" * "); break;
				case TDIV: printf(" / "); break;
				case TMOD: printf(" %% "); break;
				default: break;
			}
			opp_debug_node(base->bin_expr.right);
			break;
		}
		case ELOGIC: {
			opp_debug_node(base->logic_expr.left);
			switch (base->logic_expr.tok)
			{
				case TAND: printf(" && "); break;
				case TOR: printf(" || "); break;
				case TLT: printf(" < "); break;
				case TGT: printf(" > "); break;
				case TEQEQ: printf(" == "); break;
				case TNOTEQ: printf(" != "); break;
				case TLE: printf(" <= "); break;
				case TGE: printf(" >= "); break;
				default: break;
			}
			opp_debug_node(base->logic_expr.right);
			break;
			break;
		}
		case ECALL: {
			break;
		}

		case EASSIGN: {
			opp_debug_node(base->assign_expr.ident);

			switch (base->assign_expr.op)
			{
				case TEQ: printf(" = "); break;
				default: break;
			}

			opp_debug_node(base->assign_expr.val);
			break;
		}

		case EUNARY: {
			switch (base->unary_expr.type)
			{
				case TSTR: printf("\"%s\"", base->unary_expr.val.strval); break;
				case TIDENT: printf("%s", base->unary_expr.val.strval); break;
				case TINTEGER: printf("%lld", base->unary_expr.val.i64val); break;
				default: break;
			}
			break;
		}

		case ESUB: {
			printf("-");
			opp_debug_node(base->sub_expr.unary);
			break;
		}
		case EELEMENT: {
			break;
		}
		case EDEREF: {
			printf("*");
			opp_debug_node(base->defer_expr.defer);
			break;
		}
		case EADJUST: {
			break;
		}
		case EADDR: {
			printf("&");
			opp_debug_node(base->addr_expr.addr);
			break;
		}
		case ESIZEOF: {
			printf("sizeof");
			break;
		}
		case STMT_LABEL: {
			opp_debug_node(base->label_stmt.name);
			printf(":");
			break;
		}
		case STMT_GOTO: {
			printf("goto ");
			opp_debug_node(base->goto_stmt.name);
			break;
		}
		case STMT_IF: {
			break;
		}
		case STMT_VAR: {
			printf("auto ");
			for (unsigned int i = 0; i < base->var_stmt.vars->length; i++) {
				opp_debug_node(base->var_stmt.vars->list[i]);
				if ((i+1) < base->var_stmt.vars->length) printf(" , ");
			}
			break;
		}
		case STMT_IMPORT: {
			printf("import ");
			opp_debug_node(base->import_stmt.ident);
			break;
		}
		case STMT_WHILE: {
			break;
		}
		case STMT_FUNC: {
			break;
		}
		case STMT_RET:
		case STMT_EXTERN: 
		case STMT_FOR:
		case STMT_SWITCH: 
		case STMT_ENUM:
		case STMT_CASE: 
		case STMT_BREAK:
		break;

		default: break;
	}
}