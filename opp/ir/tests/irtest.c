// #include "../ir.h"
// #include "../../lexer/io.h"

// struct OppIr_Opcode ops[] = {
// 	{
// 		.type = OPCODE_CONST,
// 		.constant.type = IMM_I64,
// 		.constant.imm_i64 = 3
// 	},
// 	{
// 		.type = OPCODE_CONST,
// 		.constant.type = IMM_I64,
// 		.constant.imm_i64 = 1
// 	},
// 	{
// 		.type = OPCODE_CONST,
// 		.constant.type = IMM_I64,
// 		.constant.imm_i64 = 2
// 	},
// 	{
// 		.type = OPCODE_CONST,
// 		.constant.type = IMM_I64,
// 		.constant.imm_i64 = 9
// 	},
// 	{
// 		.type = OPCODE_CONST,
// 		.constant.type = IMM_I64,
// 		.constant.imm_i64 = 9
// 	},
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