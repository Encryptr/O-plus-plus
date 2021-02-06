#include "compiler.h"

// Expr
static Opp_Obj opp_compile_unary(struct Opp_Context* opp, struct Opp_Node* unary);
static Opp_Obj opp_compile_bin(struct Opp_Context* opp, struct Opp_Node* bin);
static Opp_Obj opp_compile_dot(struct Opp_Context* opp, struct Opp_Node* dot);
static Opp_Obj opp_compile_sub(struct Opp_Context* opp, struct Opp_Node* sub);
static Opp_Obj opp_compile_assign(struct Opp_Context* opp, struct Opp_Node* assign);
static Opp_Obj opp_compile_logic(struct Opp_Context* opp, struct Opp_Node* logic);
static Opp_Obj opp_compile_logic_assign(struct Opp_Context* opp, struct Opp_Node* logic);
static void opp_compile_logic_end(struct Opp_Context* opp, struct Opp_Node* logic);
static Opp_Obj opp_compile_literal(struct Opp_Context* opp, struct Opp_Node* unary);
static Opp_Obj opp_compile_addr(struct Opp_Context* opp, struct Opp_Node* expr);
static Opp_Obj opp_compile_deref(struct Opp_Context* opp, struct Opp_Node* expr);
static Opp_Obj opp_compile_elem(struct Opp_Context* opp, struct Opp_Node* expr);

Opp_Obj opp_compile_expr(struct Opp_Context* opp, struct Opp_Node* expr)
{
	switch (expr->type)
	{
		case EUNARY:
			return opp_compile_unary(opp, expr);

		case EDOT:
			return opp_compile_dot(opp, expr);

		case EADDR:
			return opp_compile_addr(opp, expr);

		case EDEREF:
			return opp_compile_deref(opp, expr);

		case EBIN:
			return opp_compile_bin(opp, expr);

		case ECALL:
			return opp_compile_call(opp, expr, true);

		case ELOGIC:
			return opp_compile_logic(opp, expr);

		case EELEMENT:
			return opp_compile_elem(opp, expr);

		default: break;
	}
}

static Opp_Obj opp_compile_struct(struct Opp_Context* opp, struct Opp_Node* unary)
{
	bool global = unary->unary_expr.bidx->type == TYPE_GLOBAL || 
		unary->unary_expr.bidx->type == TYPE_EXTERN || 
		unary->unary_expr.bidx->type == TYPE_FUNC;

	opp->info.dot.sym = unary->unary_expr.bidx;

	if (global || (unary->unary_expr.bidx->sym_type.depth > 0 && unary->unary_expr.bidx->sym_type.size == 0)) {
		opp->info.dot.inptr = true;
		OPCODE_START();
			OPCODE_TYPE(OPCODE_CONST);
			opp->ir.opcodes[opp->ir.instr_idx].constant.loc_type = IMM_U64;
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = 
				global ? IMM_SYM : IMM_LOC;

			if (global) {
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_SYM;
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_sym = 
					unary->unary_expr.bidx->key;
			}
			else {
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_LOC;
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = 
					unary->unary_expr.bidx->offset;
			}
		OPCODE_END();
	}

	if ((unary->unary_expr.bidx->sym_type.size > 0 || unary->unary_expr.bidx->sym_type.depth == 0) && !global)
		opp->info.dot.offset = unary->unary_expr.bidx->offset;

	return unary->unary_expr.bidx->sym_type;
}

static Opp_Obj opp_compile_literal(struct Opp_Context* opp, struct Opp_Node* unary)
{
	if (unary->unary_expr.bidx->sym_type.decl->t_type == TYPE_STRUCT)
		return opp_compile_struct(opp, unary);

	bool global = unary->unary_expr.bidx->type == TYPE_GLOBAL || 
		unary->unary_expr.bidx->type == TYPE_EXTERN || 
		unary->unary_expr.bidx->type == TYPE_FUNC;

	bool need_addr = unary->unary_expr.bidx->sym_type.size > 0;

	OPCODE_START();
	opp->ir.opcodes[opp->ir.instr_idx].constant.loc_type =
		opp_type_to_ir(&unary->unary_expr.bidx->sym_type);

	if (unary->unary_expr.bidx->type == TYPE_EXTERN || 
		unary->unary_expr.bidx->type == TYPE_FUNC)
		opp->ir.opcodes[opp->ir.instr_idx].constant.loc_type = IMM_U64;

	opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = 
		global ? IMM_SYM : IMM_LOC;

	if (need_addr)
		OPCODE_TYPE(OPCODE_ADDR);
	else
		OPCODE_TYPE(OPCODE_CONST);
	if (global) {
		opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_SYM;
		opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_sym = 
			unary->unary_expr.bidx->key;
		opp->ir.opcodes[opp->ir.instr_idx].constant.loc_type = IMM_U64;

		if (unary->unary_expr.bidx->type == TYPE_GLOBAL && !opp->info.need_addr) { // make sure need_addr is valud
			OPCODE_END();
			OPCODE_START();
				OPCODE_TYPE(OPCODE_DEREF);
				opp->ir.opcodes[opp->ir.instr_idx].cast.type = 
					opp_type_to_ir(&unary->unary_expr.bidx->sym_type);
		}
	}
	else {
		opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_LOC;
		opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = 
			unary->unary_expr.bidx->offset;
	}

	OPCODE_END();

	return unary->unary_expr.bidx->sym_type;
}

static Opp_Obj opp_compile_unary(struct Opp_Context* opp, struct Opp_Node* unary)
{
	if (unary->unary_expr.type == TIDENT)
		return opp_compile_literal(opp, unary);

	Opp_Obj val = {0};

	OPCODE_START();
	OPCODE_TYPE(OPCODE_CONST);

	switch (unary->unary_expr.type)
	{
		case TINTEGER: {
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_I64;
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i64 = unary->unary_expr.val.i64val;
			opp->ir.opcodes[opp->ir.instr_idx].constant.loc_type = 0;
			val.decl = int_type;
			break;
		}

		case TFLOAT: {
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_F64;
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_f64 = unary->unary_expr.val.f64val;
			opp->ir.opcodes[opp->ir.instr_idx].constant.loc_type = 0;
			val.decl = float_type;
			break;
		}

		case TSTR: {
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_STR;
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_sym = unary->unary_expr.val.strval;
			opp->ir.opcodes[opp->ir.instr_idx].constant.loc_type = IMM_U64;
			val.decl = char_type;
			val.depth = 1;
			break;
		}

		default: break;
	}

	OPCODE_END();

	return val;
}

static Opp_Obj opp_compile_bin(struct Opp_Context* opp, struct Opp_Node* bin)
{
	Opp_Obj lhs = opp_compile_expr(opp, bin->bin_expr.left);
	Opp_Obj rhs = opp_compile_expr(opp, bin->bin_expr.right);

	opp_type_cast(opp, &lhs, &rhs);

	if (lhs.depth > 0 && (bin->bin_expr.right->type != EUNARY || 
		bin->bin_expr.right->unary_expr.type != TINTEGER)) {
		OPCODE_START();
			OPCODE_TYPE(OPCODE_ARITH);
			opp->ir.opcodes[opp->ir.instr_idx].arith.type = TMUL;
			opp->ir.opcodes[opp->ir.instr_idx].arith.imm = true;
			opp->ir.opcodes[opp->ir.instr_idx].arith.val.type = IMM_I8;
			lhs.depth--;
			opp->ir.opcodes[opp->ir.instr_idx].arith.val.imm_i8 = opp_type_to_size(&lhs);
			lhs.depth++;
		OPCODE_END();
	}

	enum OppIr_Const_Type t = opp_type_to_ir(&lhs);

	OPCODE_START();
		OPCODE_TYPE(OPCODE_ARITH);
		opp->ir.opcodes[opp->ir.instr_idx].arith.type = bin->bin_expr.tok;
		opp->ir.opcodes[opp->ir.instr_idx].arith.imm = false;
		opp->ir.opcodes[opp->ir.instr_idx].arith.floating = 
			t == IMM_F64 || t == IMM_F32 ? 1 : 0;
		opp->ir.opcodes[opp->ir.instr_idx].arith.unsign = lhs.unsign;
	OPCODE_END();

	return lhs;
}

static Opp_Obj opp_compile_dot(struct Opp_Context* opp, struct Opp_Node* dot)
{
	bool outer = false;
	if (!opp->info.dot.indot) {
		opp->info.dot.indot = true;
		opp->info.dot.offset = 0;
		opp->info.dot.inptr = false;
		outer = true;
	}

	opp_compile_expr(opp, dot->dot_expr.left);

	if (opp->info.dot.inptr && dot->dot_expr.right->unary_expr.bidx->offset > 0) {
		OPCODE_START();
			OPCODE_TYPE(OPCODE_ARITH);
			opp->ir.opcodes[opp->ir.instr_idx].arith.type = TADD;
			opp->ir.opcodes[opp->ir.instr_idx].arith.imm = true;
			opp->ir.opcodes[opp->ir.instr_idx].arith.val.type = 
				(dot->dot_expr.right->unary_expr.bidx->offset > 128) 
					? IMM_I8 : IMM_I32;
			opp->ir.opcodes[opp->ir.instr_idx].arith.val.imm_i32 = 
				(int32_t)dot->dot_expr.right->unary_expr.bidx->offset;
		OPCODE_END();

		if (!outer) {
			OPCODE_START();
			OPCODE_TYPE(OPCODE_DEREF);
					opp->ir.opcodes[opp->ir.instr_idx].cast.type = 
						opp_type_to_ir(&dot->dot_expr.right->unary_expr.bidx->sym_type);
			OPCODE_END();
		}
	}
	else {
		opp->info.dot.offset += dot->dot_expr.right->unary_expr.bidx->offset;
	}

	if (outer) {
		if (!opp->info.need_addr) 
		{
			OPCODE_START();
			if (opp->info.dot.inptr) {
				OPCODE_TYPE(OPCODE_DEREF);
				opp->ir.opcodes[opp->ir.instr_idx].cast.type = 
					opp_type_to_ir(&dot->dot_expr.right->unary_expr.bidx->sym_type);
			}
			else {
				OPCODE_TYPE(OPCODE_CONST);
				opp->ir.opcodes[opp->ir.instr_idx].constant.loc_type = 
					opp_type_to_ir(&dot->dot_expr.right->unary_expr.bidx->sym_type);
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_LOC;
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = 
					opp->info.dot.offset;
			}
			OPCODE_END();
		}
		opp->info.dot.indot = false;
	}

	return dot->dot_expr.right->unary_expr.bidx->sym_type;
}

static Opp_Obj opp_compile_sub(struct Opp_Context* opp, struct Opp_Node* sub)
{

}

static Opp_Obj opp_compile_logic(struct Opp_Context* opp, struct Opp_Node* logic)
{
	Opp_Obj obj;
	bool in_parent = false;

	if (!opp->cond_state.in_logic) {
		switch (opp->cond_state.cond_type)
		{
			case LOGIC_COND: {
				opp->cond_state.endloc = opp->info.label_loc++;
				opp->cond_state.locs[0] = opp->info.label_loc++; // True
				opp->cond_state.locs[1] = opp->info.label_loc++; // False

				opp->cond_state.use_op = true;
				opp->cond_state.jloc = opp->cond_state.locs[1];
				opp->cond_state.curr_logic = 0;
				opp->cond_state.hs = 0;
				break;
			}

			case IFWHILE_COND: {
				opp->cond_state.use_op = true;
				opp->cond_state.jloc = opp->cond_state.locs[1];
				opp->cond_state.curr_logic = 0;
				opp->cond_state.hs = 0;
				break;
			}

			case SWITCH_COND: { break; }
		}
		opp->cond_state.in_logic = true;
		in_parent = true;
	}

	obj = opp_compile_logic_assign(opp, logic);

	if (in_parent) {
		opp_compile_logic_end(opp, logic);
		opp->cond_state.in_logic = false;
	}

	return obj;
}

static enum Opp_Token jmp_opposites[] = {
	[TGT] = TLE,
	[TLT] = TGE,
	[TEQEQ] = TNOTEQ,
	[TGE] = TLT,
	[TLE] = TGT,
	[TNOTEQ] = TEQEQ
};

static Opp_Obj opp_compile_logic_assign(struct Opp_Context* opp, struct Opp_Node* logic)
{
	Opp_Obj type_info = {0};
	struct Opp_Cond save = opp->cond_state;

	switch (logic->logic_expr.tok) 
	{
		case TEQEQ: case TNOTEQ: case TLT:
		case TGT: case TLE: case TGE: {
			Opp_Obj lhs = opp_compile_expr(opp, logic->logic_expr.left);
			Opp_Obj rhs = opp_compile_expr(opp, logic->logic_expr.right);

			// temp
			type_info = lhs;
			assert(lhs.unsign == 0 && rhs.unsign == 0);
			assert(lhs.decl->t_type != TYPE_FLOAT && lhs.decl->t_type != TYPE_DOUBLE);
			// check sign vs unsign
			// cast types 

			OPCODE_START();
				OPCODE_TYPE(OPCODE_CMP);
				opp->ir.opcodes[opp->ir.instr_idx].cmp.imm = 0; // add imm
			OPCODE_END();

			OPCODE_START();
				OPCODE_TYPE(OPCODE_JMP);
				if (opp->cond_state.use_op)
					opp->ir.opcodes[opp->ir.instr_idx].jmp.type = 
						jmp_opposites[logic->logic_expr.tok];
				else
					opp->ir.opcodes[opp->ir.instr_idx].jmp.type = 
						logic->logic_expr.tok;

				opp->ir.opcodes[opp->ir.instr_idx].jmp.loc = opp->cond_state.jloc;
			OPCODE_END();
			break;
		}

		case TOR: {
			opp->cond_state.curr_logic = TOR;

			if (logic->logic_expr.left->logic_expr.tok == TAND) {
				opp->cond_state.locs[1] = opp->info.label_loc++;
			}

			opp->cond_state.use_op = 0;
			opp->cond_state.jloc = opp->cond_state.locs[0];
			opp->cond_state.hs = 0;
			type_info = opp_compile_logic_assign(opp, logic->logic_expr.left);

			if (logic->logic_expr.left->logic_expr.tok == TAND) {
				OPCODE_START();
					OPCODE_TYPE(OPCODE_LABEL);
					opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32
						= opp->cond_state.locs[1];
				OPCODE_END();

				opp->cond_state.locs[1] = save.locs[1];
			}

			if (save.curr_logic == TOR && save.hs == 0) {
				opp->cond_state.use_op = 0;
				opp->cond_state.jloc = opp->cond_state.locs[0];
			}
			else if (save.hs == 1 && save.curr_logic == TOR) {
				opp->cond_state.use_op = 0;
				opp->cond_state.jloc = opp->cond_state.locs[0];
			}
			else {
				opp->cond_state.use_op = 1;
				opp->cond_state.jloc = opp->cond_state.locs[1];
			}
			opp->cond_state.hs = 1;
			opp->cond_state.curr_logic = save.curr_logic;
			opp_compile_logic_assign(opp, logic->logic_expr.right);
			break;
		}

		case TAND: {
			opp->cond_state.curr_logic = TAND;

			if (logic->logic_expr.left->logic_expr.tok == TOR) {
				opp->cond_state.locs[0] = opp->info.label_loc++;
			}

			opp->cond_state.use_op = 1;
			opp->cond_state.jloc = opp->cond_state.locs[1];
			opp->cond_state.hs = 0;
			type_info = opp_compile_logic_assign(opp, logic->logic_expr.left);

			if (logic->logic_expr.left->logic_expr.tok == TOR) {
				OPCODE_START();
					OPCODE_TYPE(OPCODE_LABEL);
					opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = opp->cond_state.locs[0];
				OPCODE_END();

				opp->cond_state.locs[0] = save.locs[0];
			}

			if (save.curr_logic == TOR && save.hs == 0) {
				opp->cond_state.use_op = 0;
				opp->cond_state.jloc = opp->cond_state.locs[0];
			}
			else if (save.hs == 1 && save.curr_logic == TOR) {
				opp->cond_state.use_op = 0;
				opp->cond_state.jloc = opp->cond_state.locs[0];
			}
			else {
				opp->cond_state.use_op = 1;
				opp->cond_state.jloc = opp->cond_state.locs[1];
			}
			opp->cond_state.hs = 1;
			opp->cond_state.curr_logic = save.curr_logic;
			opp_compile_logic_assign(opp, logic->logic_expr.right);
			break;
		}

		default: break;
	}

	opp->cond_state = save;
	return type_info;
}

static void opp_compile_logic_end(struct Opp_Context* opp, struct Opp_Node* logic)
{
	switch (opp->cond_state.cond_type) 
	{
		case LOGIC_COND: {
			OPCODE_START();
				OPCODE_TYPE(OPCODE_LABEL);
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = opp->cond_state.locs[0];
			OPCODE_END();

			OPCODE_START();
				OPCODE_TYPE(OPCODE_CONST);
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_I64;
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i64 = 1;
				opp->ir.opcodes[opp->ir.instr_idx].constant.nopush = 1;
			OPCODE_END();

			OPCODE_START();
				OPCODE_TYPE(OPCODE_JMP);
				opp->ir.opcodes[opp->ir.instr_idx].jmp.type = PURE_JMP;
				opp->ir.opcodes[opp->ir.instr_idx].jmp.loc = opp->cond_state.endloc;
			OPCODE_END();

			OPCODE_START();
				OPCODE_TYPE(OPCODE_LABEL);
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = opp->cond_state.locs[1];
			OPCODE_END();

			OPCODE_START();
				OPCODE_TYPE(OPCODE_CONST);
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_I64;
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i64 = 0;
				opp->ir.opcodes[opp->ir.instr_idx].constant.nopush = 0;
			OPCODE_END();

			OPCODE_START();
				OPCODE_TYPE(OPCODE_LABEL);
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = opp->cond_state.endloc;
			OPCODE_END();
			break;
		}

		case IFWHILE_COND:
			opp->cond_state.cond_type = LOGIC_COND;
			break;

		case SWITCH_COND:
			break;
	}
}

static Opp_Obj opp_compile_addr(struct Opp_Context* opp, struct Opp_Node* expr)
{
	Opp_Obj val = {0};
	switch (expr->addr_expr.addr->type)
	{
		case EUNARY: {
			bool global = expr->addr_expr.addr->unary_expr.bidx->type == TYPE_GLOBAL ||
				expr->addr_expr.addr->unary_expr.bidx->type == TYPE_EXTERN;

			OPCODE_START();
				if (!global)
					OPCODE_TYPE(OPCODE_ADDR);
				else
					OPCODE_TYPE(OPCODE_CONST);

				if (!global) {
					opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_LOC;
					opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = 
						expr->addr_expr.addr->unary_expr.bidx->offset;
				}
				else {
					opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_SYM;
					opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_sym = 
						expr->addr_expr.addr->unary_expr.val.strval;
				}
			OPCODE_END();

			val = expr->addr_expr.addr->unary_expr.bidx->sym_type;
			break;
		}

		case EDOT: {
			NEED_ADDR();
			val = opp_compile_expr(opp, expr->addr_expr.addr);
			REVERT();

			if (!opp->info.dot.inptr) {
				OPCODE_START();
					OPCODE_TYPE(OPCODE_ADDR);
					opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_LOC;
					opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = 
						opp->info.dot.offset;
				OPCODE_END();
			}
			break;
		}

		default: {
			NEED_ADDR();
			val = opp_compile_expr(opp, expr->addr_expr.addr);
			REVERT();
			break;
		}
	}

	val.depth++;
	return val;
}

static Opp_Obj opp_compile_deref(struct Opp_Context* opp, struct Opp_Node* expr)
{
	bool outer = false;
	if (!opp->info.in_deref) {
		outer = true;
		opp->info.in_deref = true;
	}

	Opp_Obj t = opp_compile_expr(opp, expr->deref_expr.deref);

	t.depth--;

	if (!opp->info.need_addr && t.decl->t_type != TYPE_STRUCT) {
		OPCODE_START();
			OPCODE_TYPE(OPCODE_DEREF);
			opp->ir.opcodes[opp->ir.instr_idx].cast.type = opp_type_to_ir(&t);
		OPCODE_END();
	}

	if (!outer && opp->info.need_addr && t.depth != 0) {
		OPCODE_START();
			OPCODE_TYPE(OPCODE_DEREF);
			opp->ir.opcodes[opp->ir.instr_idx].cast.type = opp_type_to_ir(&t);
		OPCODE_END();
	}

	if (outer)
		opp->info.in_deref = false;

	return t;
}

static Opp_Obj opp_compile_elem(struct Opp_Context* opp, struct Opp_Node* expr)
{
	bool save = opp->info.need_addr;
	NEED_ADDR();
	Opp_Obj n = opp_compile_expr(opp, expr->elem_expr.name);
	opp->info.need_addr = save;

	Opp_Obj temp = n;
	temp.depth = 0;
	temp.size = 0;

	if (expr->elem_expr.name->type == EDOT || n.decl->t_type == TYPE_STRUCT) {
		if (!opp->info.dot.inptr) {
			opp->info.dot.inptr = true;
			OPCODE_START();
				OPCODE_TYPE(OPCODE_ADDR);
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_LOC;
				opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i32 = 
					opp->info.dot.offset;
			OPCODE_END();
		}	
	}

	opp_compile_expr(opp, expr->elem_expr.loc);

	unsigned int size = opp_type_to_size(&temp);

	if (size > 1) {
		OPCODE_START();
			OPCODE_TYPE(OPCODE_CONST);
			opp->ir.opcodes[opp->ir.instr_idx].constant.loc_type = IMM_I64;
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.type = IMM_I64;
			opp->ir.opcodes[opp->ir.instr_idx].constant.val.imm_i64 = size;
			opp->ir.opcodes[opp->ir.instr_idx].constant.nopush = false;
		OPCODE_END();

		OPCODE_START();
			OPCODE_TYPE(OPCODE_ARITH);
			opp->ir.opcodes[opp->ir.instr_idx].arith.type = TMUL;
			opp->ir.opcodes[opp->ir.instr_idx].arith.imm = false;
			opp->ir.opcodes[opp->ir.instr_idx].arith.floating = false;
		OPCODE_END();
	}

	OPCODE_START();
		OPCODE_TYPE(OPCODE_ARITH);
		opp->ir.opcodes[opp->ir.instr_idx].arith.type = TADD;
			opp->ir.opcodes[opp->ir.instr_idx].arith.imm = false;
	OPCODE_END();

	if (!opp->info.need_addr && opp->info.dot.indot == false) { // check for any errors
		OPCODE_START();
			OPCODE_TYPE(OPCODE_DEREF);
			opp->ir.opcodes[opp->ir.instr_idx].cast.type = 
				opp_type_to_ir(&temp);
		OPCODE_END();
	}

	n.depth--;
	return n;
}