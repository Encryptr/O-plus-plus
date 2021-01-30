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

#include <sys/mman.h>
#include <stdbool.h>
#include <time.h>
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
	s->io.fsize = size;
	rewind(s->io.file);
	char* content = calloc(1, size + 2); 

	int _ = fread(content, size, 1, s->io.file);
	fclose(s->io.file);

	init_opp_lex(s, content);
}

enum {
	S_PARSER,
	S_ANALIZE,
	S_COMPILER,
	S_IR
};

typedef struct {
	clock_t start, end;
	double list[4];
} Opp_Stats;

#define START_CLOCK() { stats.start = clock(); }
#define END_CLOCK() { stats.end = clock(); }
#define GET_TIME(elem) { \
	stats.list[elem] = ((double) (stats.end - stats.start)) / CLOCKS_PER_SEC; \
}

static void opp_debug_info(Opp_Stats* s, struct Opp_Parser* opp)
{
	printf("%sOpp Compiler Debug Info\n%s", CL_GREEN, CL_RESET);
	printf("File: %s\n", opp->lex->io.fname);
	printf("Size: %ld\n", opp->lex->io.fsize);
	printf("Benchmarks: (time)\n");
	printf(" - Parser:   %lf\n", s->list[S_PARSER]);
	printf(" - Analizer: %lf\n", s->list[S_ANALIZE]);
	printf(" - Compiler: %lf\n", s->list[S_COMPILER]);
	printf(" - Ir:       %lf\n", s->list[S_IR]);
	#ifdef X86_CPU
	printf("CPU: x86-64\n");
	#else
	printf("CPU: ERROR\n");
	#endif
	#ifdef OS_ERROR
	printf("OS: ERROR\n");
	#elif defined(LINUX64)
	printf("OS: Linux64\n");
	#elif defined(MAC64)
	printf("OS: Macos64\n");
	#endif
}

void run_main(struct OppIr* ir)
{
	void *mem = mmap(NULL, 4096, PROT_WRITE | PROT_EXEC,
        32 | MAP_PRIVATE, -1, 0);

	memcpy(mem, ir->code.bytes, ir->code.idx);

	long (*run)(int) = mem;
	printf("%ld\n", run(2));
	munmap(mem, 4096);
}

void opp_init_module(const char* fname, struct Opp_Options* opts)
{
	Opp_Stats stats = {0};
	struct Opp_Scan scan = {0};

	opp_init_file(fname, &scan);

	START_CLOCK();
	struct Opp_Parser* parser = opp_parser_init(&scan);
	opp_parser_begin(parser);
	END_CLOCK();
	GET_TIME(S_PARSER);

	START_CLOCK();
	struct Opp_Analize* ctx = opp_init_analize(parser, opts);
	analize_tree(ctx);
	END_CLOCK();
	GET_TIME(S_ANALIZE);

	// struct Opp_Context* context = opp_init_compile(parser, opts);
	// opp_compile(context);

	// context->oppir = init_oppir();
	// oppir_get_opcodes(context->oppir, &context->ir);
	// oppir_eval(context->oppir);
	// OppIO io = {
	// 	.file = fopen("out.o", "wb")
	// };
	// // dump_bytes(context->oppir, &io);
	// oppir_emit_obj(context->oppir, &io);

	if (opts->debug)
		opp_debug_info(&stats, parser);
}

struct Opp_Parser* opp_add_module(const char* fname)
{
	struct Opp_Scan scan = {0};

	opp_init_file(fname, &scan);

	struct Opp_Parser* parser = opp_parser_init(&scan);
	opp_parser_begin(parser);

	return parser;
}

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
	printf("\t-w              | Hide warnings\n");
	printf("\t-debug          | Debug compiler statistics\n");
	printf("\t-Wall           | Turn warnings to errors\n");
	printf("\n");
}

static void opp_args(int argc, const char** argv, struct Opp_Options* opts)
{
	// Defaults
	opts->warning = true;
	for (int i = 1; i < argc - 1; i++) {
		if (CMP("run")) opts->run_output = true;
		else if (CMP("build")) opts->run_output = false;
		else if (CMP("-d")) opts->dump_toks  = true;
		else if (CMP("-lc")) opts->link_c    = true;
		else if (CMP("-nostd")) opts->nostd  = true;
		else if (CMP("-w")) opts->warning    = false;
		else if (CMP("-debug")) opts->debug  = true;
		else if (CMP("-Wall")) opts->wall    = true;
		else printf("Invalid argument '%s'\n", argv[i]);
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

int main(int argc, const char** argv) 
{
	struct Opp_Options opts = {0};

	if (!opp_init())
		exit(1);

	if (!allocator_init())
		INTERNAL_ERROR("Allocator init fail");

	if (argc < 2) {
		opp_help();
		return 0;
	}

	opp_args(argc, argv, &opts);
	opp_init_module(argv[argc-1], &opts);

	// objdump -b binary -D -m i386:x86-64 -M intel out.bin 

	allocator_free();

	return 0;
}