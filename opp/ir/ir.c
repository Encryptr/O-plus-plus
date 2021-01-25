/** @file ir.c
 * 
 * @brief Opp IR
 *      
 * Copyright (c) 2020 Maks S
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */ 

#include "ir.h"


static struct Register regs[] = {
	{.reg = REG_RAX, .used = 0},
	{.reg = REG_RCX, .used = 0},
	{.reg = REG_RDX, .used = 0},

	{.reg = REG_XMM0, .used = 0},
	{.reg = REG_XMM1, .used = 0},
	{.reg = REG_XMM2, .used = 0},
};

// L.123 \0
static unsigned char a_label[7];

struct OppIr* init_oppir()
{
	struct OppIr* ir = (struct OppIr*)malloc(sizeof(struct OppIr));

	if (ir == NULL)
		goto err;

	// Bytecode
	ir->code.bytes = (unsigned char*)malloc(INIT_BYTECODE_SIZE);

	if (ir->code.bytes == NULL)
		goto err;

	ir->code.idx = 0;
	ir->code.allocated = INIT_BYTECODE_SIZE;

	// Local Stack
	ir->local_stack.size = 0;
	ir->local_stack.pos = 0;
	ir->local_stack.frame_ptr = 0;

	// Offset Tables 
	ir->offsets.offset_table = (int32_t*)malloc(sizeof(int32_t)*DEFAULT_OFFSET_TABLE);
	ir->offsets.jmp_table = (struct Jmp_Item*)malloc(sizeof(struct Jmp_Item)*DEFAULT_OFFSET_TABLE);

	if (ir->offsets.offset_table == NULL || ir->offsets.jmp_table == NULL)
		goto err;

	ir->offsets.allocated = DEFAULT_OFFSET_TABLE;
	ir->offsets.jmp_idx = 0;
	memset(ir->offsets.offset_table, 0, sizeof(int32_t)*DEFAULT_OFFSET_TABLE);
	memset(ir->offsets.jmp_table, 0, sizeof(struct Jmp_Item)*DEFAULT_OFFSET_TABLE);

	// Register Allocation
	ir->regalloc.amount = 0;
	ir->regalloc.allocated = DEFAULT_SPILL;
	ir->regalloc.spills = (struct Spill*)
		malloc(sizeof(struct Spill)*DEFAULT_SPILL);
	memset(ir->regalloc.spills, 0, sizeof(struct Spill)*DEFAULT_SPILL);

	if (ir->regalloc.spills == NULL)
		goto err;

	// Reg Stack
	ir->reg_stack.stack = (struct Register*)
		malloc(sizeof(struct Register)*DEFAULT_REG_STACK);
	memset(ir->reg_stack.stack, 0, sizeof(struct Register)*DEFAULT_REG_STACK);

	if (ir->reg_stack.stack == NULL)
		goto err;

	ir->reg_stack.top = ir->reg_stack.stack;
	ir->reg_stack.allocated = DEFAULT_REG_STACK;
	ir->instr = NULL;

	// Data
	ir->data_seg.data = (unsigned char*)
		malloc(INIT_BYTECODE_SIZE);

	if (ir->data_seg.data == NULL)
		goto err;

	ir->data_seg.allocated = INIT_BYTECODE_SIZE;
	ir->data_seg.idx = 0;

	memset(ir->data_seg.data, 0, INIT_BYTECODE_SIZE);

	// End
	return ir;

	err:
		INTERNAL_ERROR("Malloc Fail");
		return NULL;
}

void oppir_free(struct OppIr* ir)
{
	free(ir->code.bytes);
	free(ir->offsets.offset_table);
	free(ir->offsets.jmp_table);
	free(ir->regalloc.spills);
	free(ir->reg_stack.stack);
	free(ir);
}

void dump_bytes(struct OppIr* ir, OppIO* io)
{
	if (io == NULL) {
		for (size_t i = 0; i < ir->code.idx; i++) { 
			if ((i+1) % 16 == 0) printf("\n");
			else printf("%X ", ir->code.bytes[i]);
		}
		printf("\n");
	}
	else {
		fwrite(ir->code.bytes, sizeof(unsigned char), ir->code.idx, io->file);
		fclose(io->file);
	}
}

void oppir_get_opcodes(struct OppIr *ir, struct OppIr_Instr* instr)
{
	if (instr == NULL)
		return;

	ir->instr = instr;
}

void oppir_eval(struct OppIr* ir)
{
	#ifdef LINUX64
	init_strtab();
	init_elf_syms();
	init_reloc();
	#endif

	for (size_t index = 0; index < ir->instr->instr_idx; index++)
		oppir_eval_opcode(ir, &ir->instr->opcodes[index]);

	#ifdef LINUX64
	init_elf_header(DEFAULT_SECT);
	init_text_sect(ir);
	init_data_sect(ir);
	init_symtab_sect();
	init_shstrtab_sect();
	init_strtab_sect();
	init_rela_text_sect();
	elf_offsets(ir);
	#endif

	dump_data(ir);
}

void oppir_emit_obj(struct OppIr* ir, OppIO* out)
{
	#ifdef LINUX64
	write_elf64(ir, out);
	#endif
}

void oppir_check_realloc(struct OppIr* ir, unsigned int bytes)
{
	if ((ir->code.idx + bytes) >= ir->code.allocated) {

		ir->code.bytes = (unsigned char*)
				realloc(ir->code.bytes, (64+ir->code.allocated));

		if (ir->code.bytes == NULL)
			INTERNAL_ERROR("Malloc Fail");

		ir->code.allocated += 64;
	}
}

void oppir_check_data(struct OppIr* ir, unsigned int bytes)
{
	if ((ir->data_seg.idx + bytes) >= ir->data_seg.allocated) {

		ir->data_seg.data = (unsigned char*)
				realloc(ir->data_seg.data, (64+ir->data_seg.allocated));

		if (ir->data_seg.data == NULL)
			INTERNAL_ERROR("Malloc Fail");

		ir->data_seg.allocated += 64;
	}
}

static void oppir_gen_label()
{
	static unsigned int idx = 0;
	sprintf(a_label, "L.%u", idx);
	idx++;
}

static void oppir_write_data(struct OppIr* ir, const unsigned int len, unsigned char* d)
{
	oppir_check_data(ir, len+1);
	for (unsigned int i = 0; i <= len; i++)
		ir->data_seg.data[ir->data_seg.idx++] = d[i];
}

static void oppir_check_regstack(struct OppIr* ir)
{
	if (ir->reg_stack.top >= (ir->reg_stack.stack + ir->reg_stack.allocated)) {
		ir->reg_stack.stack = (struct Register*)
				realloc(ir->reg_stack.stack, (16+ir->reg_stack.allocated));

		if (ir->reg_stack.stack == NULL)
			INTERNAL_ERROR("Malloc fail");

		ir->reg_stack.allocated += 16;
	}
}

static int32_t oppir_get_spill(struct OppIr* ir)
{
	for (unsigned int i = 0; i < ir->regalloc.allocated; i++) {
		if (!ir->regalloc.spills[i].use && ir->regalloc.spills[i].made) {
			ir->regalloc.spills[i].use = 1;
			return ir->regalloc.spills[i].loc;
		}
	}

	if (ir->regalloc.amount == ir->regalloc.allocated)
		INTERNAL_ERROR("OppIR spill limit met");

	ir->local_stack.size += 8;
	ir->local_stack.pos -= 8;
	
	ir->regalloc.spills[ir->regalloc.amount].loc = ir->local_stack.pos;
	ir->regalloc.spills[ir->regalloc.amount].use = 1;
	ir->regalloc.spills[ir->regalloc.amount].made = 1;
	ir->regalloc.amount++;

	return ir->local_stack.pos;
}

static enum Regs oppir_push_reg(struct OppIr* ir, enum OppIr_Const_Type type)
{
	int i = (type == IMM_F32 || type == IMM_F64) ? 3 : 0;
	const int max = (i == 3) ? 6 : 3;

	for (i; i < max; i++) {
		if (!regs[i].used) {
			regs[i].used = 1;
			oppir_check_regstack(ir);
			ir->reg_stack.top->reg = regs[i].reg;
			ir->reg_stack.top->type = type;
			ir->reg_stack.top->used = 1;
			ir->reg_stack.top->loc = 0; 
			ir->reg_stack.top->spilled = 0;
			ir->reg_stack.top++;
			return regs[i].reg;
		}
	}

	return oppir_reg_alloc(ir);
}

static void oppir_push(struct OppIr* ir, struct Register reg)
{
	if (regs[reg.reg].used == 0) {
		regs[reg.reg].used = 1;
		oppir_check_regstack(ir);
		*ir->reg_stack.top = reg;
		ir->reg_stack.top->used = 1;
		ir->reg_stack.top->loc = 0;
		ir->reg_stack.top->spilled = 0;
		ir->reg_stack.top++;
	}
	else
		printf("Internal error oppir_push\n");
}

static enum Regs oppir_reg_alloc(struct OppIr* ir)
{
	struct Register spill_reg = {0};
	struct OppIr_Set set;

	for (struct Register* i = ir->reg_stack.stack; i < ir->reg_stack.top; i++) {
		if (i->used && !i->spilled) {
			i->spilled = 1;
			i->loc = oppir_get_spill(ir);
			spill_reg.loc = i->loc;
			spill_reg.reg = i->reg;
			spill_reg.type = i->type;

			oppir_check_regstack(ir);
			ir->reg_stack.top->reg = i->reg;
			ir->reg_stack.top->used = 1;
			ir->reg_stack.top->loc = 0;
			ir->reg_stack.top->spilled = 0;
			ir->reg_stack.top++;
			break;
		}
	}

	set.val.type = IMM_I32;
	set.val.imm_i32 = spill_reg.loc;
	set.loc_type = spill_reg.type;
	set.global = 0;

	oppir_save_reg(ir, spill_reg.reg, &set);

	return spill_reg.reg;
}

static struct Register oppir_pop_reg(struct OppIr* ir)
{
	struct Register pop_reg = {0};
	ir->reg_stack.top--;

	if (ir->reg_stack.top < ir->reg_stack.stack || !ir->reg_stack.top->used)
		printf("Error reg_stack pop overflow\n");

	if (ir->reg_stack.top->spilled) {
		ir->reg_stack.top->used = 0;
		ir->reg_stack.top->spilled = 0;
		struct OppIr_Const val = {
			.global = 0,
			.loc_type = ir->reg_stack.top->type,
			.val.type = IMM_LOC,
			.val.imm_i32 = ir->reg_stack.top->loc
		};
		
		oppir_eval_const(ir, &val);
		pop_reg = oppir_pop_reg(ir);

		for (unsigned int i = 0; i < ir->regalloc.allocated; i++) {
			if (ir->regalloc.spills[i].loc == ir->reg_stack.top->loc) {
				ir->regalloc.spills[i].use = 0;
				break;
			}
		}
	}
	else {
		ir->reg_stack.top->used = 0;
		pop_reg = *ir->reg_stack.top;
		regs[pop_reg.reg].used = 0;
	}

	return pop_reg;
}

static void oppir_write_const(struct OppIr* ir, struct OppIr_Value* imm)
{
	switch (imm->type)
	{
		case IMM_U64:
		case IMM_I64: 
		{
			oppir_check_realloc(ir, 8); 
			ir->code.bytes[ir->code.idx++] = imm->imm_i64 & 0xFF;
			for (int i = 8; i <= 56; i += 8) 
				ir->code.bytes[ir->code.idx++] = (imm->imm_i64 >> i) & 0xFF;
			break;
		}

		case IMM_I32: {
			oppir_check_realloc(ir, 4); 
			ir->code.bytes[ir->code.idx++] = imm->imm_i32 & 0xFF;
			for (int i = 8; i <= 24; i += 8) 
				ir->code.bytes[ir->code.idx++] = (imm->imm_i32 >> i) & 0xFF;
			break;
		}

		case IMM_I8: {
			oppir_check_realloc(ir, 1);
			ir->code.bytes[ir->code.idx++] = imm->imm_i8;
			break;
		}

		default: break;
	}
}

void oppir_eval_opcode(struct OppIr* ir, struct OppIr_Opcode* op) 
{
	switch (op->type)
	{
		case OPCODE_CONST:
			oppir_eval_const(ir, &op->constant);
			break;
		
		case OPCODE_ASSIGN:
			oppir_eval_set(ir, &op->set);
			break;

		case OPCODE_FUNC:
			oppir_eval_func(ir, &op->func);
			break;

		case OPCODE_ARG:
			oppir_eval_arg(ir, &op->arg);
			break;

		case OPCODE_END:
			oppir_eval_end(ir);
			break;

		case OPCODE_CAST:
			oppir_eval_cast(ir, &op->cast);
			break;

		case OPCODE_ADDR:
			oppir_eval_addr(ir, &op->constant);
			break;

		case OPCODE_DEREF:
			oppir_eval_deref(ir, &op->cast);
			break;

		case OPCODE_PTR_ASSIGN:
			oppir_eval_ptr_assign(ir, &op->cast);
			break;

		case OPCODE_CALL:
			oppir_eval_call(ir, &op->call);
			break;

		case OPCODE_MOV_ARG:
			oppir_eval_mov_arg(ir, &op->arg);
			break;
		// case OPCODE_LABEL:
		// 	oppir_eval_label(ir, &op->constant);
		// 	break;

		// case OPCODE_JMP:
		// 	oppir_eval_jmp(ir, &op->jmp);
		// 	break;

		// case OPCODE_CMP:
		// 	oppir_eval_cmp(ir, &op->cmp);
		// 	break;

		case OPCODE_VAR:
			oppir_eval_var(ir, &op->var);
			break;

		case OPCODE_RET:
			oppir_eval_ret(ir);
			break;

		case OPCODE_ARITH:
			oppir_eval_arith(ir, &op->arith);
			break;

		// case OPCODE_BIT:
		// 	oppir_eval_bitwise(ir, &op->bit);
		// 	break;

		// case OPCODE_CALL: break;
		// 	// oppir_eval_call(ir, &op->)

		default: break;
	}
}

static void oppir_eval_sym(struct OppIr* ir, struct OppIr_Const* imm)
{

	// add types put in separet func
	// oppir_check_realloc(ir, 7);
	// IR_EMIT(0x48); IR_EMIT(0x8b);
	// IR_EMIT(0x05 + (reg_type*8));
	// IR_EMIT(0x00); IR_EMIT(0x00);
	// IR_EMIT(0x00); IR_EMIT(0x00);


	// after this make deref of reg with size
	// add reallocation to ir_linxu module
}

static void oppir_eval_const(struct OppIr* ir, struct OppIr_Const* imm)
{
	enum Regs reg_type;

	if (!imm->nopush) {
		enum OppIr_Const_Type t;

		if (imm->loc_type == 0)
			t = imm->val.type;
		else
			t = imm->loc_type;

		reg_type = oppir_push_reg(ir, t);
	}
	else
		reg_type = REG_RAX;

	if (imm->val.type == IMM_SYM) {
		oppir_eval_sym(ir, imm);
	}
	else if (imm->val.type > IMM_LOC && imm->val.type < IMM_F32) {
		oppir_check_realloc(ir, 2);
		imm->val.type = IMM_I64;
		ir->code.bytes[ir->code.idx++] = 0x48;
		ir->code.bytes[ir->code.idx++] = 0xb8 + reg_type;
	}
	else if (imm->val.type > IMM_I64) {
		assert(false);
		printf("ADD FLOATs\n");
	}
	else if (imm->val.type == IMM_LOC)
		oppir_write_reg(ir, reg_type, imm);
	else if (imm->val.type == IMM_STR) {
		oppir_gen_label();
		make_global_sym((char*)a_label, ir->data_seg.idx, 0);
		oppir_write_data(ir, strlen(imm->val.imm_sym), imm->val.imm_sym);
		oppir_check_realloc(ir, 7);
		IR_EMIT(0x48);
		IR_EMIT(0x8d);
		IR_EMIT(0x05 + (reg_type*8));
		make_reloc(ir->code.idx, (char*)a_label, 2);
		imm->val.type = IMM_I32;
		imm->val.imm_i32 = 0;
	}

	oppir_write_const(ir, &imm->val);
}

static void oppir_write_reg(struct OppIr* ir, enum Regs reg_type, struct OppIr_Const* imm)
{
	bool small = imm->val.imm_i32 > -128;

	if (small)
		imm->val.type = IMM_I8;
	else
		imm->val.type = IMM_I32;

	oppir_check_realloc(ir, 8);
	switch (imm->loc_type)
	{
		case IMM_F32:
		case IMM_F64:
			if (imm->loc_type == IMM_F32)
				IR_EMIT(0xf3);
			else
				IR_EMIT(0xf2);
			IR_EMIT(0x0f);
			IR_EMIT(0x10);
			reg_type -= 3;
			break;

		case IMM_U8:
		case IMM_U16:
		case IMM_I8:
		case IMM_I16:
		case IMM_I32:
			IR_EMIT(0x48);
			if (imm->loc_type != IMM_I32)
				IR_EMIT(0x0f);
			else
				IR_EMIT(0x63);

			switch (imm->loc_type) {
					break;
				case IMM_I8: 
					IR_EMIT(0xbe); 
					break;
				case IMM_U8:
					IR_EMIT(0xb7);
					break;
				case IMM_I16:
					IR_EMIT(0xbf);
					break;
				case IMM_U16:
					IR_EMIT(0xb7);
					break;
			}
			break;

		case IMM_U64:
		case IMM_I64:
			IR_EMIT(0x48);
			IR_EMIT(0x8b);
			break;

		case IMM_U32:
			IR_EMIT(0x8b);
			break;

		default: break;
	}
	if (small) 
		IR_EMIT(0x45 + (reg_type*8));
	else 
		IR_EMIT(0x85 + (reg_type*8));
}

static void oppir_save_reg(struct OppIr* ir, enum Regs reg_type, struct OppIr_Set* set)
{
	bool small = set->val.imm_i32 > -128;

	if (small)
		set->val.type = IMM_I8;

	oppir_check_realloc(ir, 3);
	switch (set->loc_type)
	{
		case IMM_I8:
		case IMM_U8:
			IR_EMIT(0x88);
			break;

		case IMM_I16:
		case IMM_U16:
			IR_EMIT(0x66);
			IR_EMIT(0x89);
			break;

		case IMM_I32:
		case IMM_U32:
			IR_EMIT(0x89);
			break;

		case IMM_I64:
		case IMM_U64:
			IR_EMIT(0x48);
			IR_EMIT(0x89);
			break;

		case IMM_F32:
			IR_EMIT(0xf3);
			IR_EMIT(0x0f);
			IR_EMIT(0x11);
			reg_type -= 3;
			break;

		case IMM_F64:
			IR_EMIT(0xf2);
			IR_EMIT(0x0f);
			IR_EMIT(0x11);
			reg_type -= 3;
			break;
	}

	if (small)
		IR_EMIT(0x45 + (reg_type*8));
	else
		IR_EMIT(0x85 + (reg_type*8));

	oppir_write_const(ir, &set->val);
}

static void oppir_eval_set(struct OppIr* ir, struct OppIr_Set* set)
{
	assert(set->global != 1);

	struct Register reg_type = oppir_pop_reg(ir);
	bool small = set->val.imm_i32 > -128;

	oppir_save_reg(ir, reg_type.reg, set);
}

static void oppir_emit_frame(struct OppIr* ir) 
{
	struct OppIr_Value frame = {
		.type = IMM_I32,
		.imm_i32 = 0x00000000
	};

	IR_EMIT(0x55); IR_EMIT(0x48); 
	IR_EMIT(0x89); IR_EMIT(0xe5);

	IR_EMIT(0x48); IR_EMIT(0x81); IR_EMIT(0xec);
	ir->local_stack.frame_ptr = ir->code.idx;

	oppir_write_const(ir, &frame);
}

static void oppir_eval_func(struct OppIr* ir, struct OppIr_Func* fn)
{
	if (fn->ext) {
		make_extern(fn->fn_name);
		return;
	}

	oppir_check_realloc(ir, 11);

	// Reset local func info
	ir->local_stack.size = 0;
	ir->local_stack.pos = 0;
	ir->offsets.jmp_idx = 0;
	ir->regalloc.amount = 0;
	ir->reg_stack.top = ir->reg_stack.stack;

	for (unsigned int i = 0; i < ir->regalloc.allocated; i++) {
		ir->regalloc.spills[i].made = 0;
		ir->regalloc.spills[i].use = 0;
	}

	#ifdef LINUX64
	make_fn_sym(fn->fn_name, ir->code.idx);
	#endif

	oppir_emit_frame(ir);
}

static void oppir_eval_arg(struct OppIr* ir, struct OppIr_Arg* arg)
{
	// Ignore size type for now all are treated as 64 bit regs

	switch (arg->type)
	{
		case IMM_U8...IMM_I64:
			oppir_check_realloc(ir, 4);
			if (arg->idx < 4)
				IR_EMIT(0x48);
			else
				IR_EMIT(0x4c);

			IR_EMIT(0x89);

			if (arg->idx <= 1)
				IR_EMIT(0x7d - (arg->idx*8));
			else if (arg->idx <= 3)
				IR_EMIT(0x55 - ((arg->idx-2)*8));
			else
				IR_EMIT(0x45 + ((arg->idx-4)*8));
			break;

		case IMM_F32...IMM_F64:
			oppir_check_realloc(ir, 5);
			if (arg->type == IMM_F64)
				IR_EMIT(0xf2);
			else
				IR_EMIT(0xf3);

			IR_EMIT(0x0f);
			IR_EMIT(0x11);
			IR_EMIT(0x45 + (arg->idx*8));
			break;
		default: break;
	}
	ir->local_stack.size += 8;
	ir->local_stack.pos -= 8;
	IR_EMIT((char)ir->local_stack.pos);
}

static void oppir_eval_end(struct OppIr* ir)
{
	struct OppIr_Value stack_size = {
		.type = IMM_I32,
		.imm_i32 = ir->local_stack.size
	};

	size_t temp = ir->code.idx;
	ir->code.idx = ir->local_stack.frame_ptr;

	while ((stack_size.imm_i32 % 16) != 0)
		stack_size.imm_i32 += 1;

	oppir_write_const(ir, &stack_size);

	ir->code.idx = temp;

	IR_EMIT(0xc9); 
	IR_EMIT(0xc3);

	oppir_set_offsets(ir);
}

static void oppir_set_offsets(struct OppIr* ir)
{
	// struct OppIr_Const val = {
	// 	.type = IMM_I32,
	// 	.imm_i32 = 0
	// };

	// for (unsigned int i = 0; i < ir->offsets.jmp_idx; i++) {
	// 	size_t temp = ir->code.idx;

	// 	int32_t jmp_loc = ir->offsets.offset_table[ir->offsets.jmp_table[i].table_pos] 
	// 		- (ir->offsets.jmp_table[i].loc + 4);

	// 	ir->code.idx = ir->offsets.jmp_table[i].loc;
	// 	val.imm_i32 = jmp_loc;
	// 	oppir_write_const(ir, &val);

	// 	ir->code.idx = temp;
	// }
}

static void oppir_cast_to_f32(struct OppIr* ir, enum Regs reg)
{
	enum Regs new_reg = oppir_push_reg(ir, IMM_F32);
	switch (reg)
	{
		case REG_RAX...REG_RDX:
			oppir_check_realloc(ir, 5);

			IR_EMIT(0xf3); IR_EMIT(0x48);
			IR_EMIT(0x0f); IR_EMIT(0x2a);
			IR_EMIT((0xc0 + ((new_reg-3)*8)) + reg);
			break;

		case REG_XMM0...REG_XMM2:
			oppir_check_realloc(ir, 4);

			IR_EMIT(0xf2); IR_EMIT(0x0f);
			IR_EMIT(0x5a);
			IR_EMIT(0xc0 + ((new_reg-3)*8) + (reg-3));
			break;
	}
}

static void oppir_cast_to_f64(struct OppIr* ir, enum Regs reg)
{
	enum Regs new_reg = oppir_push_reg(ir, IMM_F64);
	switch (reg)
	{
		case REG_RAX...REG_RDX:
			oppir_check_realloc(ir, 5);

			IR_EMIT(0xf2); IR_EMIT(0x48);
			IR_EMIT(0x0f); IR_EMIT(0x2a);

			IR_EMIT((0xc0 + ((new_reg-3)*8)) + (reg));
			break;

		case REG_XMM0...REG_XMM2:
			oppir_check_realloc(ir, 4);

			IR_EMIT(0xf3); IR_EMIT(0x0f);
			IR_EMIT(0x5a);
			IR_EMIT(0xc0 + ((new_reg-3)*8) + (reg-3));
			break;
	}
}

static void oppir_eval_cast(struct OppIr* ir, struct OppIr_Cast* cast)
{
	struct Register reg = oppir_pop_reg(ir);

	switch (cast->type)
	{
		case IMM_F32:
			oppir_cast_to_f32(ir, reg.reg);
			break;

		case IMM_F64:
			oppir_cast_to_f64(ir, reg.reg);
			break;

		default:
			break;
	}
}

static void oppir_eval_var(struct OppIr* ir, struct OppIr_Var* var)
{
	if (var->global) {
		make_global_sym(var->name, ir->data_seg.idx, 0);
		oppir_check_data(ir, var->size);
		ir->data_seg.idx += var->size;
	}
	else {
		ir->local_stack.size += var->size;
		ir->local_stack.pos -= var->size;
	}
}

static void oppir_eval_addr(struct OppIr* ir, struct OppIr_Const* addr)
{
	enum Regs reg = oppir_push_reg(ir, IMM_U64);

	oppir_check_realloc(ir, 7);

	if (addr->global) {
		assert(false);
		return;
	}

	bool small = addr->val.imm_i32 > -128;
	if (small)
		addr->val.type = IMM_I8;
	else
		addr->val.type = IMM_I32;

	IR_EMIT(0x48);
	IR_EMIT(0x8d);

	if (small)
		IR_EMIT(0x45 + (reg*8));
	else 
		IR_EMIT(0x85 + (reg*8));

	oppir_write_const(ir, &addr->val);
}

static void oppir_emit_reg_comb(struct OppIr* ir, enum Regs lhs, enum Regs rhs)
{
	switch (lhs)
	{
		case REG_RAX: IR_EMIT(0xc0 + (rhs*8)); break;
		case REG_RCX: IR_EMIT(0xc1 + (rhs*8)); break;
		case REG_RDX: IR_EMIT(0xc2 + (rhs*8)); break;
		default: break;
	}
}

static void oppir_eval_arith(struct OppIr* ir, struct OppIr_Arith* arith)
{
	struct Register rhs = oppir_pop_reg(ir);
	BLOCK_REG(rhs.reg);
	struct Register lhs;

	if (!arith->imm)
		lhs = oppir_pop_reg(ir);
	else 
		lhs = rhs;

	UNBLOCK_REG(rhs.reg);

	assert(rhs.type != IMM_F32 && rhs.type != IMM_F64);

	oppir_check_realloc(ir, 7);

	IR_EMIT(0x48);
	switch (arith->type)
	{
		case TADD: {
			if (arith->imm) {
				if (arith->val.type == IMM_I8) {
					IR_EMIT(0x83);
					IR_EMIT(0xc0 + rhs.reg);
				}
				else {
					if (rhs.type == REG_RAX)
						IR_EMIT(0x05);
					else {
						IR_EMIT(0x81);
						IR_EMIT(0xc0 + rhs.reg);
					}
				}
			}
			else
				IR_EMIT(0x1);
			break;
		}

		case TMIN: {
			break;
		}

		case TMUL: {
			if (arith->imm) {
				if (arith->val.imm_i8 == -1) {
					IR_EMIT(0xf7);
					IR_EMIT(0xd8 + rhs.reg);
					goto skip;
				}
				else {
					// for shifting we need to add sign vs unsign
					IR_EMIT(0xc1);
					IR_EMIT(0xe0 + rhs.reg);
					char b = 0;
					while (arith->val.imm_i8 != 1)
						arith->val.imm_i8 >>= 1, b++;
					arith->val.imm_i8 = b;
				}
			}
			else {
				IR_EMIT(0x0f);
				IR_EMIT(0xaf);
				oppir_emit_reg_comb(ir, rhs.reg, lhs.reg);
				goto skip;
			}
			break;
		}
	}

	if (arith->imm)
		oppir_write_const(ir, &arith->val);
	else
		oppir_emit_reg_comb(ir, lhs.reg, rhs.reg);

	skip:
	oppir_push(ir, lhs);
}

static void oppir_eval_deref(struct OppIr* ir, struct OppIr_Cast* cast)
{
	struct Register reg = oppir_pop_reg(ir);

	oppir_check_realloc(ir, 4);

	assert(cast->type != IMM_F32 && cast->type != IMM_F64);

	switch (cast->type)
	{
		case IMM_I8:
			IR_EMIT(0x48);
			IR_EMIT(0x0f);
			IR_EMIT(0xbe);
			break;

		case IMM_U8:
			IR_EMIT(0x48);
			IR_EMIT(0x0f);
			IR_EMIT(0xb6);
			break;

		case IMM_I16:
			IR_EMIT(0x48);
			IR_EMIT(0x0f);
			IR_EMIT(0xbf);
			break;

		case IMM_U16:
			IR_EMIT(0x48);
			IR_EMIT(0x0f);
			IR_EMIT(0xb7);
			break;

		case IMM_I32:
			IR_EMIT(0x48);
			IR_EMIT(0x63);
			break;

		case IMM_U32:
			IR_EMIT(0x8b);
			break;

		case IMM_I64:
		case IMM_U64:
			IR_EMIT(0x48);
			IR_EMIT(0x8b);
			break;

		default: break;
	}
	IR_EMIT(0x9 * (reg.reg));

	oppir_push(ir, reg);	
}

static void oppir_eval_ptr_assign(struct OppIr* ir, struct OppIr_Cast* cast)
{
	struct Register rhs = oppir_pop_reg(ir);
	struct Register lhs = oppir_pop_reg(ir);

	assert(cast->type != IMM_F32 && cast->type != IMM_F64);

	oppir_check_realloc(ir, 3);

	switch (cast->type)
	{
		case IMM_I8:
		case IMM_U8:
			IR_EMIT(0x88);
			break;

		case IMM_I16:
		case IMM_U16:
			IR_EMIT(0x66);
			IR_EMIT(0x89);
			break;

		case IMM_I32:
		case IMM_U32:
			IR_EMIT(0x89);
			break;

		case IMM_I64:
		case IMM_U64:
			IR_EMIT(0x48);
			IR_EMIT(0x89);
			break;

		default: break;
	}

	switch (lhs.reg)
	{
		case REG_RAX:
			IR_EMIT(0x08 * (rhs.reg));
			break;

		case REG_RCX:
			IR_EMIT(rhs.reg==REG_RAX ? 0x1 : 0x11);
			break;

		case REG_RDX:
			IR_EMIT(rhs.reg==REG_RAX ? 0x2 : 0xa);
			break;
	}
}

static void oppir_eval_ret(struct OppIr* ir)
{
	struct Register reg = oppir_pop_reg(ir);

	if (reg.reg == REG_RAX || reg.reg == REG_XMM0)
		return;

	switch (reg.reg)
	{
		case REG_RCX:
		case REG_RDX:
			oppir_check_realloc(ir, 3);
			IR_EMIT(0x48);
			IR_EMIT(0x89);
			IR_EMIT(0xc0 + (reg.reg*8));
			break;

		default:
			oppir_check_realloc(ir, 4);
			if (reg.type == IMM_F32)
				IR_EMIT(0xf3);
			else
				IR_EMIT(0xf2);

			IR_EMIT(0x10);
			IR_EMIT(0xc1 + (reg.reg-4));
			break;
	}
}

static void oppir_eval_call(struct OppIr* ir, struct OppIr_Call* call)
{
	struct Register reg;

	// Spill all
	for (struct Register* i = ir->reg_stack.stack; i < ir->reg_stack.top; i++) {
		if (i->used && !i->spilled) {
			printf("SPILL\n");
		}
	}

	if (call->imm) {
		oppir_check_realloc(ir, 4);
		IR_EMIT(0xe8);
		make_reloc(ir->code.idx, call->name, 4);
		IR_EMIT(0x00);
		IR_EMIT(0x00);
		IR_EMIT(0x00);
		IR_EMIT(0x00);
	}
	else {
		assert(false);
	}

	if (call->ret) {
		oppir_push(ir, ((struct Register){.reg = REG_RAX, .type = call->ret_type}));
	}
}

static void oppir_eval_mov_arg(struct OppIr* ir, struct OppIr_Arg* arg)
{
	struct Register reg = oppir_pop_reg(ir);

	assert(arg->idx == 0);

	IR_EMIT(0x48); IR_EMIT(0x89);
	IR_EMIT(0xc7);
}