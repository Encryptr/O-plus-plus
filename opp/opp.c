/** @file opp.c
 * 
 * @brief Opp Main Module
 *      
 * Copyright (c) 2021 Maks S
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
**/

#include <stdio.h>
#include <time.h>
#include "./memory/memory.h"
#include "./parser/parser.h"
#include "./preprocessor/preprocessor.h"
#include "./semantics/analizer.h"
#include "./lexer/lexer.h"
#include "./util/util.h"
#include "opp.h"

struct Opp_State global_state = {0};

#define CMP(a) !strcmp(argv[i], a)
#define OPP_VERISION "0.0.1"

enum {
	S_PARSER,
	S_ANALIZE,
	S_COMPILER,
};

typedef struct {
	clock_t start, end;
	double list[3];
} Opp_Stats;

#define START_CLOCK() { stats.start = clock(); }
#define END_CLOCK() { stats.end = clock(); }
#define GET_TIME(elem) \
	stats.list[elem] = ((double) (stats.end - stats.start)) / CLOCKS_PER_SEC

struct Opp_Compiler {
	struct Opp_Scan* scan;
	struct Opp_Parser* parser;
};

static void opp_debug_info(Opp_Stats* s, struct Opp_Compiler* comp)
{
	printf("%s### Opp Interpreter Debug Info ###\n%s", CL_GREEN, CL_RESET);
	printf("File: %s\n", comp->scan->io.fname);
	printf("Size: %ld\n", comp->scan->io.fsize);
	printf("Benchmarks:\n");
	printf(" - Parser:   %lf\n", s->list[S_PARSER]);
	printf(" - Analizer: %lf\n", s->list[S_ANALIZE]);
	printf(" - Compiler: %lf\n", s->list[S_COMPILER]);
	printf("Total: %lf\n", s->list[S_PARSER] + s->list[S_ANALIZE] 
		+ s->list[S_COMPILER]);
	#ifdef X86_CPU
	printf("ARCH: x86-64\n");
	#else
	printf("ARCH: ERROR\n");
	#endif
	#ifdef OS_ERROR
	printf("OS: ERROR\n");
	#elif defined(LINUX64)
	printf("OS: Linux64\n");
	#elif defined(MAC64)
	printf("OS: macOS64\n");
	#endif
	printf("%s################################%s\n", CL_GREEN, CL_RESET);
}

static void opp_init_module(const char* fname, struct Opp_State* state)
{
	Opp_Stats stats = {0};
	struct Opp_Compiler comp = {0};

	int res = setjmp(state->error_buf);

	if (res != OPP_ERROR) {
		allocator_init(malloc, free);

		// fname = opp_invoke_preproc(fname);

		START_CLOCK();
		comp.scan = opp_init_lex(fname);
		comp.parser = opp_init_parser(comp.scan);
		opp_parser_begin(comp.parser);
		END_CLOCK();
		GET_TIME(S_PARSER);

		START_CLOCK();
		opp_start_analyzer(comp.parser);
		END_CLOCK();
		GET_TIME(S_ANALIZE);
		
		if (state->opts.debug)
			opp_debug_info(&stats, &comp);
		if (state->opts.dump_toks)
			dump_tokens(comp.scan);
		if (state->opts.preproc)
			printf("%s\n", comp.scan->content);
	}
	allocator_free();
	// opp_cleanup(fname);
}

static void opp_help()
{
	printf("\x1b[32mO++ Interpreter\x1b[0m\n");
	printf("\x1b[32mVersion: %s\x1b[0m\n", OPP_VERISION);
	printf("\n\x1b[31mFormat: opp [flags] [file]\x1b[0m\n");
	printf("\t-c              | Check source code for errors (without running)\n");
	printf("\t-d              | For dumping file tokens\n");
	printf("\t-w              | Hide warnings\n");
	printf("\t-E              | Dump preprocessor file\n");
	printf("\t-debug          | Debug compiler statistics\n");
	printf("\t-Wall           | Turn warnings to errors\n");
	printf("\n");
}

static void opp_args(int argc, const char** argv)
{
	// Defaults
	global_state.opts.warning = true;
	for (int i = 1; i < argc - 1; i++) {
		if (CMP("-d")) global_state.opts.dump_toks       = true;
		else if (CMP("-w")) global_state.opts.warning    = false;
		else if (CMP("-debug")) global_state.opts.debug  = true;
		else if (CMP("-Wall")) global_state.opts.wall    = true;
		else if (CMP("-E")) global_state.opts.preproc    = true;
		else printf("%sInvalid argument '%s'%s\n", CL_RED, argv[i], CL_RESET);
	}
}

int main(int argc, const char** argv)
{
	if (argc < 2) {
		opp_help();
		return 1;
	}

	opp_args(argc, argv);
	opp_init_module(argv[argc - 1], &global_state);

	return 0;
}
