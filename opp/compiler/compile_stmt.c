#include "compiler.h"

// Stmts
static void opp_compile_ret(struct Opp_Context* opp, struct Opp_Node* node);
static void opp_compile_bitfield(struct Opp_Context* opp, struct Opp_Node* node);
static void opp_compile_block(struct Opp_Context* opp, struct Opp_Node* node);
static void opp_compile_label(struct Opp_Context* opp, struct Opp_Node* node);
static void opp_compile_goto(struct Opp_Context* opp, struct Opp_Node* node);
static void opp_compile_assign(struct Opp_Context* opp, struct Opp_Node* node);
static void opp_compile_if(struct Opp_Context* opp, struct Opp_Node* node);
static void opp_compile_while(struct Opp_Context* opp, struct Opp_Node* node);
static void opp_compile_for(struct Opp_Context* opp, struct Opp_Node* node);
static void opp_compile_switch(struct Opp_Context* opp, struct Opp_Node* node);
static void opp_compile_break(struct Opp_Context* opp, struct Opp_Node* node);

void opp_compile_stmt(struct Opp_Context* opp, struct Opp_Node* node)
{
	switch (node->type)
	{
		case STMT_VAR:
			opp_compile_var(opp, node);
			break;

		case STMT_BLOCK:
			opp_compile_block(opp, node);
			break;

		case EASSIGN:
			opp_compile_assign(opp, node);
			break;

		case ECALL:
			opp_compile_call(opp, node, false);
			break;

		case STMT_RET:
			opp_compile_ret(opp, node);
			break;

		case STMT_IF:
			opp_compile_if(opp, node);
			break;

		case STMT_WHILE:
			opp_compile_while(opp, node);
			break;

		case STMT_BREAK:
			opp_compile_break(opp, node);
			break;

		default: 
			// temp
			opp_compile_expr(opp, node);
		break;
	}
}

void opp_compile_var(struct Opp_Context* opp, struct Opp_Node* node)
{
	struct Opp_Stmt_Var* var = &node->var_stmt;

	bool assign = var->var->type == EASSIGN ? 1 : 0;
	bool global = var->bidx->type == TYPE_GLOBAL || var->bidx->type == TYPE_EXTERN;
	unsigned int size = opp_type_to_size(&var->type);

	OPCODE_START();
		OPCODE_TYPE(OPCODE_VAR);
		opp->ir.opcodes[opp->ir.instr_idx].var.global = global;
		opp->ir.opcodes[opp->ir.instr_idx].var.size = size;
		opp->ir.opcodes[opp->ir.instr_idx].var.name = var->bidx->key;
	OPCODE_END();

	if (!global) {
		opp->info.stack_offset -= size;
		var->bidx->offset = opp->info.stack_offset;
	}

	if (assign)
		opp_compile_assign(opp, var->var);
}

Opp_Obj opp_compile_call(struct Opp_Context* opp, struct Opp_Node* call, bool stmt)
{
	assert(call->call_expr.callee->type != EDOT);

	if (call->call_expr.callee->type == EUNARY) {
		struct Opp_Bucket* b = call->call_expr.callee->unary_expr.bidx;

		for (int i = call->call_expr.args->length-1; i >= 0; i--) {
			Opp_Obj t = opp_compile_expr(opp, call->call_expr.args->list[i]);

			opp_type_cast(opp, &b->args->args[i].var_stmt.type, &t);
		}
		
		OPCODE_START();
			OPCODE_TYPE(OPCODE_MOV_ARG);
			opp->ir.opcodes[opp->ir.instr_idx].arg.stop = true;
		OPCODE_END();

		int Iidx = 0;
		int Fidx = 0;
		for (int i = 0; i < call->call_expr.args->length; i++) {
			int idx = 0;

			if ((b->args->args[i].var_stmt.type.decl->t_type == TYPE_FLOAT || 
				b->args->args[i].var_stmt.type.decl->t_type == TYPE_DOUBLE) &&
				b->args->args[i].var_stmt.type.depth == 0)
				idx = Fidx++;
			else
				idx = Iidx++;


			OPCODE_START();
				OPCODE_TYPE(OPCODE_MOV_ARG);
				opp->ir.opcodes[opp->ir.instr_idx].arg.idx = idx;
				opp->ir.opcodes[opp->ir.instr_idx].arg.type = 
					opp_type_to_ir(&b->args->args[i].var_stmt.type);
			OPCODE_END();
		}

		OPCODE_START();
			OPCODE_TYPE(OPCODE_CALL);
			opp->ir.opcodes[opp->ir.instr_idx].call.name = 
				call->call_expr.callee->unary_expr.val.strval;
			opp->ir.opcodes[opp->ir.instr_idx].call.imm = 1;
			if (b->sym_type.decl->t_type == TYPE_VOID) {
				opp->ir.opcodes[opp->ir.instr_idx].call.ret = 0;
			}
			else {
				opp->ir.opcodes[opp->ir.instr_idx].call.ret = stmt;
				opp->ir.opcodes[opp->ir.instr_idx].call.ret_type = 
					opp_type_to_ir(&b->sym_type);
			}
			opp->ir.opcodes[opp->ir.instr_idx].call.extra_args = 
				call->call_expr.args->length;
		OPCODE_END();

		return b->sym_type;
	}
}

static void opp_compile_block(struct Opp_Context* opp, struct Opp_Node* node)
{
	for (unsigned int i = 0; i < node->block_stmt.len; i++)
		opp_compile_stmt(opp, node->block_stmt.stmts[i]);
}

static void opp_assign_dot(struct Opp_Context* opp, struct Opp_Expr_Assign* assign)
{
	NEED_ADDR();
	Opp_Obj lhs = opp_compile_expr(opp, assign->ident);
	REVERT();

	Opp_Obj rhs = opp_compile_expr(opp, assign->val);
	
	opp_type_cast(opp, &lhs, &rhs);

	OPCODE_START();
		if (opp->info.dot.inptr) {
			OPCODE_TYPE(OPCODE_PTR_ASSIGN);
			opp->ir.opcodes[opp->ir.instr_idx].cast.type = opp_type_to_ir(&lhs);
		}
		else {
			OPCODE_TYPE(OPCODE_ASSIGN);
			opp->ir.opcodes[opp->ir.instr_idx].set.loc_type = opp_type_to_ir(&lhs);
			opp->ir.opcodes[opp->ir.instr_idx].set.val.type = IMM_I32;
			opp->ir.opcodes[opp->ir.instr_idx].set.val.imm_i32 = 
				(int32_t)opp->info.dot.offset;
		}
	OPCODE_END();		
}

static void opp_compile_ptr_assign(struct Opp_Context* opp, struct Opp_Expr_Assign* assign)
{
	NEED_ADDR();
	Opp_Obj lhs = opp_compile_expr(opp, assign->ident);
	REVERT();

	Opp_Obj rhs = opp_compile_expr(opp, assign->val);

	opp_type_cast(opp, &lhs, &rhs);

	OPCODE_START();
		OPCODE_TYPE(OPCODE_PTR_ASSIGN);
		opp->ir.opcodes[opp->ir.instr_idx].cast.type = opp_type_to_ir(&lhs);
	OPCODE_END();
}

static void opp_compile_assign(struct Opp_Context* opp, struct Opp_Node* node)
{
	struct Opp_Expr_Assign* assign = &node->assign_expr;

	switch (assign->ident->type)
	{
		case EDEREF:
		case EELEMENT:
			opp_compile_ptr_assign(opp, assign);
			return;
			break;

		case EDOT:
			opp_assign_dot(opp, assign);
			return;
			break;

		default: break;
	}

	bool global = assign->ident->unary_expr.bidx->type == TYPE_GLOBAL || 
			assign->ident->unary_expr.bidx->type == TYPE_EXTERN;

	if (global) {
		NEED_ADDR();
		opp_compile_expr(opp, assign->ident);
		REVERT();
	}

	Opp_Obj rhs = opp_compile_expr(opp, assign->val);
	Opp_Obj* lhs = &assign->ident->unary_expr.bidx->sym_type;

	opp_type_cast(opp, lhs, &rhs);
	
	OPCODE_START();
		OPCODE_TYPE(OPCODE_ASSIGN);
		opp->ir.opcodes[opp->ir.instr_idx].set.loc_type = opp_type_to_ir(lhs);
		if (global) {
			opp->ir.opcodes[opp->ir.instr_idx].set.global = true;
			opp->ir.opcodes[opp->ir.instr_idx].set.val.type = IMM_SYM;
			opp->ir.opcodes[opp->ir.instr_idx].set.val.imm_sym = 
				assign->ident->unary_expr.val.strval;
		}
		else {
			opp->ir.opcodes[opp->ir.instr_idx].set.val.type = IMM_I32;
			opp->ir.opcodes[opp->ir.instr_idx].set.val.imm_i32 = 
				assign->ident->unary_expr.bidx->offset;
		}
	OPCODE_END();
}

static void opp_compile_ret(struct Opp_Context* opp, struct Opp_Node* node)
{
	Opp_Obj ret = opp_compile_expr(opp, node->ret_stmt.value);

	opp_type_cast(opp, &opp->info.ret_type, &ret);

	OPCODE_START();
		OPCODE_TYPE(OPCODE_RET);
	OPCODE_END();
}

static void opp_compile_if(struct Opp_Context* opp, struct Opp_Node* node)
{
	struct Opp_Stmt_If* ifstmt = &node->if_stmt;

	bool has_else = ifstmt->other != NULL;

	if (ifstmt->cond->type == EUNARY && ifstmt->cond->unary_expr.type == TINTEGER) {
		if (ifstmt->cond->unary_expr.val.i64val == 1)
			opp_compile_stmt(opp, ifstmt->then);
		else {
			if (has_else)
				opp_compile_stmt(opp, ifstmt->other);
		}

		return;
	}

	opp->cond_state.cond_type = IFWHILE_COND;
	unsigned int end, body, other;

	end = opp->info.label_loc++;
	body = opp->info.label_loc++;
	if (has_else) 
		other = opp->info.label_loc++;
	else
		other = end;

	opp->cond_state.endloc = end;
	opp->cond_state.locs[0] = body;
	opp->cond_state.locs[1] = other;

	opp_compile_expr(opp, ifstmt->cond);

	OPCODE_START();
		OPCODE_TYPE(OPCODE_LABEL);
		opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = body;
	OPCODE_END();

	opp_compile_stmt(opp, ifstmt->then);

	if (has_else) {
		OPCODE_START();
			OPCODE_TYPE(OPCODE_JMP);
			opp->ir.opcodes[opp->ir.instr_idx].jmp.type = PURE_JMP;
			opp->ir.opcodes[opp->ir.instr_idx].jmp.loc = end;
		OPCODE_END();

		OPCODE_START();
			OPCODE_TYPE(OPCODE_LABEL);
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = other;
		OPCODE_END();
		opp_compile_stmt(opp, ifstmt->other);
	}

	OPCODE_START();
		OPCODE_TYPE(OPCODE_LABEL);
		opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = end;
	OPCODE_END();
}

static void opp_compile_inifite(struct Opp_Context* opp, struct Opp_Node* node)
{ // might not work with break
	unsigned int start = opp->info.label_loc++;

	OPCODE_START();
		OPCODE_TYPE(OPCODE_LABEL);
		opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = start;
	OPCODE_END();

	opp_compile_stmt(opp, node->while_stmt.then);

	OPCODE_START();
		OPCODE_TYPE(OPCODE_JMP);
		opp->ir.opcodes[opp->ir.instr_idx].jmp.type = PURE_JMP;
		opp->ir.opcodes[opp->ir.instr_idx].jmp.loc = start;
	OPCODE_END();
}

static void opp_compile_while(struct Opp_Context* opp, struct Opp_Node* node)
{
	struct Opp_Stmt_While* loop = &node->while_stmt;

	if (loop->cond->type == EUNARY && loop->cond->unary_expr.type == TINTEGER) {
		if (loop->cond->unary_expr.val.i64val == 0)
			return;
		else {
			opp_compile_inifite(opp, node);
			return;
		}
	}

	unsigned int start, body, end;

	opp->cond_state.cond_type = IFWHILE_COND;

	start = opp->info.label_loc++;
	body  = opp->info.label_loc++;
	end   = opp->info.label_loc++;

	opp->cond_state.endloc = end;
	opp->cond_state.locs[0] = body;
	opp->cond_state.locs[1] = end;

	OPCODE_START();
		OPCODE_TYPE(OPCODE_LABEL);
		opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = start;
	OPCODE_END();

	opp_compile_expr(opp, loop->cond);

	OPCODE_START();
		OPCODE_TYPE(OPCODE_LABEL);
		opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = body;
	OPCODE_END();

	opp_compile_stmt(opp, loop->then);

	OPCODE_START();
		OPCODE_TYPE(OPCODE_JMP);
		opp->ir.opcodes[opp->ir.instr_idx].jmp.type = PURE_JMP;
		opp->ir.opcodes[opp->ir.instr_idx].jmp.loc = start;
	OPCODE_END();

	OPCODE_START();
		OPCODE_TYPE(OPCODE_LABEL);
		opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = end;
	OPCODE_END();
}

static void opp_compile_break(struct Opp_Context* opp, struct Opp_Node* node)
{
	OPCODE_START();
		OPCODE_TYPE(OPCODE_JMP);
		opp->ir.opcodes[opp->ir.instr_idx].jmp.type = PURE_JMP;
		opp->ir.opcodes[opp->ir.instr_idx].jmp.loc = opp->cond_state.endloc;
	OPCODE_END();
}