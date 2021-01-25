// #include "../ir.h"
// #include "../../lexer/io.h"

// //gcc ../../lexer/lexer.c ../ir-linux.c ../ir.c irtest.c 

// struct OppIr_Opcode ops[] = {
// 	{
// 		.type = OPCODE_VAR,
// 		.var.global = 1,
// 		.var.name = "test",
// 		.var.size = 8,
// 		.var.sym = 3
// 	},
// 	{
// 		.type = OPCODE_FUNC,
// 		.func.fn_name = "main",
// 		.func.sym = 2
// 	},

// 	{
// 		.type = OPCODE_END
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