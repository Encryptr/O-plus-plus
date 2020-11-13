/** @file opp.c
 * 
 * @brief Main Opp compiler file
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

#include <stdbool.h>
#include "opp.h"

void opp_init_file(const char* fname, struct Opp_Scan* s)
{
	long size;
	s->io.file = fopen(fname, "r");
	s->io.fname = fname;

	if (!s->io.file)
		printf("\"%s\" Is not a file...\n", fname), exit(1);

	fseek(s->io.file, 0, SEEK_END);
	size = ftell(s->io.file);
	rewind(s->io.file);
	char* content = calloc(1, size + 2); 

	fread(content, size, 1, s->io.file);
	fclose(s->io.file);

	init_opp_lex(s, content);
}

void opp_init_module(const char* fname, struct Opp_Options* opts)
{
	struct Opp_Scan scan = {0};

	opp_init_file(fname, &scan);

	struct Opp_Parser* parser = opp_parser_init(&scan);
	opp_parser_begin(parser);

	struct Opp_Context* context = opp_init_compile(parser, opts);
	opp_compile(context);

	// Move to a func
	context->oppir = init_oppir();
	oppir_get_opcodes(context->oppir, &context->ir);
	oppir_eval(context->oppir);

	OppIO io = {
		.file = fopen("out.bin", "wb")
	};
	dump_bytes(context->oppir, &io);
}

// void opp_new_module(const char* fname, struct Opp_Context* opp)
// {
// 	// struct Opp_Scan scan = {0};
// 	// struct Opp_Parser* old_parser = opp->parser;

// 	// bopp_init_file(fname, &scan);

// 	// struct Opp_Parser* parser = opp_parser_init(&scan);
// 	// opp_parser_begin(parser);

// 	// opp->parser = parser;

// 	// bopp_semantic_analize(parser);

// 	// opp_compile(opp);

// 	// opp->parser = old_parser;
// }

#define CMP(a) !strcmp(argv[i], a)
#define OPP_VERISION "0.0.1"

static void opp_help()
{
	printf("\x1b[32mO++ Programming Language\x1b[0m\n");
	printf("\x1b[32mVersion: %s\x1b[0m\n", OPP_VERISION);
	printf("\n\x1b[31mFormat: opp [flags] [file]\x1b[0m\n");
	printf("\trun             | Generate executable and run\n");
	printf("\tbuild (default) | Generate obj file from source\n");
	printf("\t-d              | For dumping file tokens\n");
	printf("\t-lc             | Link C's standard library\n");
	printf("\t-nostd          | Don't link Opp stdlib\n");
	printf("\n");
}

static void opp_args(int argc, const char** argv, struct Opp_Options* opts)
{
	for (int i = 1; i < argc; i++) {
		if (CMP("run")) opts->run_output = true;
		else if (CMP("build")) opts->run_output = false;
		else if (CMP("-d")) opts->dump_toks  = true;
		else if (CMP("-lc")) opts->link_c    = true;
		else if (CMP("-nostd")) opts->nostd  = true;
	}
}

static bool opp_init()
{
	#ifndef X86_CPU
	printf("%sOpp currently only supports the x86-64 architecture%s\n",
		CL_RED, CL_RESET);
	return false;
	#endif

	#ifdef OS_ERROR
	printf("%sOpp currently only supports:\n\t- Linux (64 bit)%s\n",
		CL_RED, CL_RESET);
	return false;
	#endif

	return true;
}

#define SIZE 5
struct OppIr_Opcode instruct[] = {
	{
		.type = OPCODE_CONST,
		.constant.type = IMM_I64,
		.constant.imm_i64 = 1
	},
	{
		.type = OPCODE_CONST,
		.constant.type = IMM_I64,
		.constant.imm_i64 = 1
	},
	{
		.type = OPCODE_CONST,
		.constant.type = IMM_I64,
		.constant.imm_i64 = 1
	},
	{
		.type = OPCODE_CONST,
		.constant.type = IMM_I64,
		.constant.imm_i64 = 2
	},
	{
		.type = OPCODE_CONST,
		.constant.type = IMM_I64,
		.constant.imm_i64 = 2
	},
};


int main(int argc, const char** argv) 
{
	struct Opp_Options* opts = {0};

	if (!opp_init())
		exit(1);

	if (!allocator_init())
		INTERNAL_ERROR("Allocator init fail");

	if (argc < 2) {
		opp_help();
		return 0;
	}

	opp_args(argc, argv, opts);
	opp_init_module(argv[argc-1], opts);

	// OppIO io = {
	// 	.file = fopen("out.bin", "wb")
	// };

	// struct OppIr* ir = init_oppir();
	// struct OppIr_Instr instr = {
	// 	.opcodes = instruct,
	// 	.instr_idx = SIZE
	// };
	// oppir_get_opcodes(ir, &instr);
	// oppir_eval(ir);
	// dump_bytes(ir, &io); // objdump -b binary -D -m i386:x86-64 out.bin 

	allocator_free();

	return 0;
} 