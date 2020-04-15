/* File for WASM Port */

#ifndef OPP_WASM
#define OPP_WASM

#include <stdint.h>

enum Value_Type {
	VT_I32 = 0x7F, VT_F32 = 0x7D,
	VT_I64 = 0x7E, VT_F64 = 0x7C
};

enum Opcodes {
	OP_BLOCK = 0x02,
	OP_LOOP = 0x03,
	OP_BR = 0x0c,
	OP_BR_IF = 0x0d,
	OP_END = 0x0b,
	OP_CALL = 0x10,
	OP_GET_LOCAL = 0x20,
	OP_SET_LOCAL = 0x21,
};

enum Opcodes32 {
	I32_CONST = 0x41,
	F32_CONST = 0x43,
	I32_EQZ = 0x45,

	I32_EQ = 0x46,
	F32_EQ = 0x5b,

	F32_LT = 0x5d,
	F32_GT = 0x5e,

	I32_AND = 0x71,
	F32_AND = 0x92,

	F32_SUB = 0x93,
	F32_MUL = 0x94,
	F32_DIV = 0x95,
};

// enum Opcodes64 {

// };

const uint32_t WASM_MAGIC_NUMBER[3] = {0x00, 0x61, 0x73, 0x6d};
const uint32_t WASM_VERSION[3] = {0x01, 0x00, 0x00, 0x00};


#endif

