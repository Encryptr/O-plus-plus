// #include "../ir.h"
// #include "../../lexer/io.h"

// struct OppIr_Opcode ops[] = {
// 	{
// 		.type = OPCODE_CONST,
// 		.constant.val.type = IMM_LOC,
// 		.constant.val.imm_i32 = -26,
// 		.constant.loc_type = IMM_I8,
// 		.constant.nopush = 0
// 	},
// 	{
// 		.type = OPCODE_CONST,
// 		.constant.val.type = IMM_LOC,
// 		.constant.val.imm_i32 = -26,
// 		.constant.loc_type = IMM_I8,
// 		.constant.nopush = 0
// 	},
// 	{
// 		.type = OPCODE_CONST,
// 		.constant.val.type = IMM_LOC,
// 		.constant.val.imm_i32 = -26,
// 		.constant.loc_type = IMM_I8,
// 		.constant.nopush = 0
// 	},
// 	{
// 		.type = OPCODE_CONST,
// 		.constant.val.type = IMM_LOC,
// 		.constant.val.imm_i32 = -26,
// 		.constant.loc_type = IMM_I8,
// 		.constant.nopush = 0
// 	},
// 	// {
// 	// 	.type = OPCODE_ASSIGN,
// 	// 	.set.val.type = IMM_I32,
// 	// 	.set.val.imm_i32 = -26,
// 	// 	.set.loc_type = IMM_I8,
// 	// 	.set.global = 0
// 	// },
// 	// {
// 	// 	.type = OPCODE_ASSIGN,
// 	// 	.set.val.type = IMM_I32,
// 	// 	.set.val.imm_i32 = -26,
// 	// 	.set.loc_type = IMM_F64,
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
// 		.file = fopen("out.bin", "wb")
// 	};
// 	dump_bytes(ir, &io);
// }