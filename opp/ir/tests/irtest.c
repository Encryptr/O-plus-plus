// #include "../ir.h"
// #include "../../lexer/io.h"

// //gcc ../../lexer/lexer.c ../ir-linux.c ../ir.c irtest.c 

// struct OppIr_Opcode ops[] = {
// 	{
// 		.type = OPCODE_CONST,
// 		.constant.val.type = IMM_LOC,
// 		.constant.val.imm_i32 = -26,
// 		.constant.loc_type = IMM_U32,
// 		.constant.nopush = 0
// 	}
// 	// {
// 	// 	.type = OPCODE_CONST,
// 	// 	.constant.val.type = IMM_LOC,
// 	// 	.constant.val.imm_i32 = -26,
// 	// 	.constant.loc_type = IMM_I8,
// 	// 	.constant.nopush = 0
// 	// },

// 	// {
// 	// 	.type = OPCODE_CAST,
// 	// 	.cast.type = IMM_F64
// 	// },
// 	// {
// 	// 	.type = OPCODE_ASSIGN,
// 	// 	.set.val.type = IMM_I32,
// 	// 	.set.val.imm_i32 = -26,
// 	// 	.set.loc_type = IMM_F32,
// 	// 	.set.global = 0
// 	// },

// };

// int main() 
// {
// 	struct OppIr_Instr instr = {
// 		.instr_idx = sizeof(ops)/sizeof(ops[1]),
// 		.opcodes = ops
// 	};

// 	struct OppIr* ir = init_oppir();
// 	oppir_get_opcodes(ir, &instr);
// 	oppir_eval(ir);

// 	OppIO io = {
// 		.file = fopen("out.o", "wb")
// 	};
// 	oppir_emit_obj(ir, &io);
// }