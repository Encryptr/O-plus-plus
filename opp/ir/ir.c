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
	{.reg = REG_RDX, .used = 0}
};

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

	// Register stack / allocation
	ir->regalloc.amount = 0;
	ir->regalloc.allocated = DEFAULT_SPILL;
	ir->regalloc.spills = (struct Spill*)
		malloc(sizeof(struct Spill)*DEFAULT_SPILL);
	memset(ir->regalloc.spills, 0, sizeof(struct Spill)*DEFAULT_SPILL);

	if (ir->regalloc.spills == NULL)
		goto err;

	ir->reg_stack.stack = (struct Register*)
		malloc(sizeof(struct Register)*DEFAULT_REG_STACK);
	memset(ir->reg_stack.stack, 0, sizeof(struct Register)*DEFAULT_REG_STACK);

	if (ir->reg_stack.stack == NULL)
		goto err;

	ir->reg_stack.top = ir->reg_stack.stack;
	ir->instr = NULL;

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
	for (size_t index = 0; index < ir->instr->instr_idx; index++) {
		oppir_eval_opcode(ir, &ir->instr->opcodes[index]);
	}
}

static void oppir_check_realloc(struct OppIr* ir, unsigned int bytes)
{
	if ((ir->code.idx + bytes) >= ir->code.allocated) {

		ir->code.bytes = (unsigned char*)
				realloc(ir->code.bytes, (64+ir->code.allocated));

		if (ir->code.bytes == NULL)
			INTERNAL_ERROR("Malloc Fail");

		ir->code.allocated += 64;
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

	if (ir->regalloc.amount == ir->regalloc.allocated) {
		printf("REALLOC SPILL\n");
	}

	ir->local_stack.size += 8;
	ir->local_stack.pos -= 8;
	
	ir->regalloc.spills[ir->regalloc.amount].loc = ir->local_stack.pos;
	ir->regalloc.spills[ir->regalloc.amount].use = 1;
	ir->regalloc.spills[ir->regalloc.amount].made = 1;
	ir->regalloc.amount++;

	return ir->local_stack.pos;
}

static enum Regs oppir_push_reg(struct OppIr* ir)
{
	for (int i = 0; i < REG_COUNT; i++) {
		if (!regs[i].used) {
			regs[i].used = 1;

			ir->reg_stack.top->reg = regs[i].reg;
			ir->reg_stack.top->used = 1;
			ir->reg_stack.top->loc = 0; 
			ir->reg_stack.top->spilled = 0;
			ir->reg_stack.top++;
			return regs[i].reg;
		}
	}

	return oppir_reg_alloc(ir);
}

static enum Regs oppir_reg_alloc(struct OppIr* ir)
{
	struct Register spill_reg = {0};
	struct OppIr_Const val = {0};
	unsigned char reg_op = 0;

	for (struct Register* i = ir->reg_stack.stack; i < ir->reg_stack.top; i++) {
		if (i->used && !i->spilled) {
			i->spilled = 1;
			i->loc = oppir_get_spill(ir);
			spill_reg.loc = i->loc;
			spill_reg.reg = i->reg;

			ir->reg_stack.top->reg = i->reg;
			ir->reg_stack.top->used = 1;
			ir->reg_stack.top->loc = 0;
			ir->reg_stack.top->spilled = 0;
			ir->reg_stack.top++;
			break;
		}
	}

	oppir_check_realloc(ir, 3);
	ir->code.bytes[ir->code.idx++] = 0x48;
	ir->code.bytes[ir->code.idx++] = 0x89;
	val.type = IMM_I8;
	val.imm_i8 = (char)spill_reg.loc;

	reg_op = 0x45 + (spill_reg.reg*8);

	if (spill_reg.loc < -255) {
		val.type = IMM_I32;
		val.imm_i32 = spill_reg.loc;
		reg_op = 0x85 + (spill_reg.reg*8);
	}

	ir->code.bytes[ir->code.idx++] = reg_op;
	oppir_write_const(ir, &val);

	return spill_reg.reg;
}

static enum Regs oppir_pop_reg(struct OppIr* ir)
{
	enum Regs pop_reg;
	ir->reg_stack.top--;

	if (ir->reg_stack.top < ir->reg_stack.stack || !ir->reg_stack.top->used)
		printf("Error reg_stack pop overflow\n");

	if (ir->reg_stack.top->spilled) {
		printf("HANDLE SPILLED\n");
		// unspill reg
	}
	else {
		ir->reg_stack.top->used = 0;
		pop_reg = ir->reg_stack.top->reg;
		regs[pop_reg].used = 0;
	}

	return pop_reg;
}

static void oppir_write_const(struct OppIr* ir, struct OppIr_Const* imm)
{
	switch (imm->type)
	{
		case IMM_I64:
			oppir_check_realloc(ir, 8); 
			ir->code.bytes[ir->code.idx++] = imm->imm_i64 & 0xFF;
			for (int i = 8; i <= 56; i += 8) 
				ir->code.bytes[ir->code.idx++] = (imm->imm_i64 >> i) & 0xFF;
			break;

		case IMM_I32:
			oppir_check_realloc(ir, 4); 
			ir->code.bytes[ir->code.idx++] = imm->imm_i32 & 0xFF;
			for (int i = 8; i <= 24; i += 8) 
				ir->code.bytes[ir->code.idx++] = (imm->imm_i32 >> i) & 0xFF;
			break;

		case IMM_U32:
			oppir_check_realloc(ir, 4); 
			ir->code.bytes[ir->code.idx++] = imm->imm_u32 & 0xFF;
			for (int i = 8; i <= 24; i += 8) 
				ir->code.bytes[ir->code.idx++] = (imm->imm_u32 >> i) & 0xFF;
			break;


		case IMM_I8:
			oppir_check_realloc(ir, 1); 
			ir->code.bytes[ir->code.idx++] = imm->imm_i8;
			break;
	}
}

void oppir_eval_opcode(struct OppIr* ir, struct OppIr_Opcode* op) 
{
	switch (op->type)
	{
		case OPCODE_CONST:
			oppir_eval_const(ir, &op->constant);
			break;

		case OPCODE_FUNC:
			oppir_eval_func(ir, &op->func);
			break;

		case OPCODE_END:
			oppir_eval_end(ir);
			break;

		case OPCODE_LABEL:
			oppir_eval_label(ir, &op->constant);
			break;

		case OPCODE_JMP:
			oppir_eval_jmp(ir, &op->jmp);
			break;

		case OPCODE_CMP:
			oppir_eval_cmp(ir, &op->cmp);
			break;

		case OPCODE_VAR:
			oppir_eval_var(ir, &op->var);
			break;

		case OPCODE_ASSIGN:
			oppir_eval_set(ir, &op->set);
			break;

		case OPCODE_CALL: break;
			// oppir_eval_call(ir, &op->)
	}
}

static void oppir_eval_const(struct OppIr* ir, struct OppIr_Const* imm)
{
	enum Regs reg_type;
	if (!imm->nopush)
		reg_type = oppir_push_reg(ir);
	else 
		reg_type = REG_RAX;

	if (imm->type != IMM_STR && imm->type != IMM_LOC) {
		oppir_check_realloc(ir, 8+2);
		ir->code.bytes[ir->code.idx++] = 0x48;
		ir->code.bytes[ir->code.idx++] = 0xb8 + reg_type;
	}
	else if (imm->type == IMM_LOC) {
		oppir_check_realloc(ir, 7);
		IR_EMIT(0x48);
		IR_EMIT(0x8b);

		if (imm->imm_i32 < -255)
			IR_EMIT(0x85 + (reg_type*8));
		else {
			imm->type = IMM_I8;
			IR_EMIT(0x45 + (reg_type*8));
		}
	}

	oppir_write_const(ir, imm); 
}

static void oppir_emit_frame(struct OppIr* ir) 
{
	struct OppIr_Const frame = {
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
	oppir_check_realloc(ir, 11);

	// Reset local func info
	ir->local_stack.size = 0;
	ir->local_stack.pos = 0;
	ir->regalloc.amount = 0;
	ir->offsets.jmp_idx = 0;

	oppir_emit_frame(ir);
	oppir_local_param(ir, fn->args);
}

static void oppir_set_offsets(struct OppIr* ir)
{
	struct OppIr_Const val = {
		.type = IMM_I32,
		.imm_i32 = 0
	};

	for (unsigned int i = 0; i < ir->offsets.jmp_idx; i++) {
		size_t temp = ir->code.idx;

		int32_t jmp_loc = ir->offsets.offset_table[ir->offsets.jmp_table[i].table_pos] 
			- (ir->offsets.jmp_table[i].loc + 4);

		ir->code.idx = ir->offsets.jmp_table[i].loc;
		val.imm_i32 = jmp_loc;
		oppir_write_const(ir, &val);

		ir->code.idx = temp;
	}
}

static void oppir_eval_end(struct OppIr* ir)
{
	struct OppIr_Const stack_size = {
		.type = IMM_U32,
		.imm_u32 = ir->local_stack.size
	};

	size_t temp = ir->code.idx;
	ir->code.idx = ir->local_stack.frame_ptr;

	stack_size.imm_u32 += stack_size.imm_u32 % 16;

	oppir_write_const(ir, &stack_size);

	ir->code.idx = temp;

	IR_EMIT(0xc9); 
	IR_EMIT(0xc3);

	oppir_set_offsets(ir);
}

static void oppir_local_param(struct OppIr* ir, unsigned int args)
{
	oppir_check_realloc(ir, (args+1) * 3);

	for (unsigned int i = 0; i < args; i++) {
		if (i < 2) {
			IR_EMIT(0x48);
			IR_EMIT(0x89);
			IR_EMIT(0x7d - (i*8));
		}
		else if (i >= 2 && i < 4) {
			IR_EMIT(0x48);
			IR_EMIT(0x89);
			IR_EMIT(0x55 - (i-2)*8);
		}
		else {
			IR_EMIT(0x4c);
			IR_EMIT(0x89);
			IR_EMIT(0x4d - ((i-4)*8));
		}

		ir->local_stack.size += 8;
		ir->local_stack.pos -= 8;

		IR_EMIT(ir->local_stack.pos);
	}
}

static void oppir_eval_label(struct OppIr* ir, struct OppIr_Const* loc)
{
	if (loc->imm_u32 >= ir->offsets.allocated) {
		printf("LABEL REALLOC NEEDED\n");
	}

	ir->offsets.offset_table[loc->imm_u32] = ir->code.idx;
}

static void oppir_eval_jmp(struct OppIr* ir, struct OppIr_Jmp* jmp)
{
	if (ir->offsets.jmp_idx >= ir->offsets.allocated) {
		printf("JMP REALLOC NEEDED\n");
	}

	if (jmp->type == RET_JMP)
		oppir_pop_reg(ir);
	else if (jmp->type == PURE_JMP)
		IR_EMIT(0xe9);
	else
		IR_EMIT(0x0f);

	switch (jmp->type)
	{
		case TEQEQ: IR_EMIT(0x84); break;
		case TNOTEQ: IR_EMIT(0x85); break;
	}
	ir->offsets.jmp_table[ir->offsets.jmp_idx].loc = ir->code.idx;
	ir->offsets.jmp_table[ir->offsets.jmp_idx].table_pos = jmp->loc;
	IR_EMIT(0x00);
	IR_EMIT(0x00);
	IR_EMIT(0x00);
	IR_EMIT(0x00);

	ir->offsets.jmp_idx++;
}

static void oppir_eval_var(struct OppIr* ir, struct OppIr_Var* var)
{
	
}

static void oppir_eval_cmp(struct OppIr* ir, struct OppIr_Cmp* cmp)
{
	// OPTIMIZE IMM TODO!!
	enum Regs lhs = oppir_pop_reg(ir);
	enum Regs rhs = oppir_pop_reg(ir);

	oppir_check_realloc(ir, 3);
	IR_EMIT(0x48); IR_EMIT(0x39); IR_EMIT(0xc8);
}

static void oppir_eval_set(struct OppIr* ir, struct OppIr_Set* set)
{
	/// handle assign
}