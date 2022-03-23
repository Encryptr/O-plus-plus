/** @file opp.c
 * 
 * @brief Opp Entry Point
 *      
 * Copyright (c) 2022 Maks S
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
#include <stdio.h>
#include "./memory/memory.h"
#include "./util/util.h"
#include "opp.h"

#include "./memory/gc.h"
#include "./lexer/lexer.h"
#include "./parser/parser.h"
#include "./vm/vm.h"

#include <time.h>
#include <unistd.h>

enum { S_PARSER, S_VM };

struct Stats {
    clock_t start_time, end_time;
    double times[2];
};

#define START_CLOCK() do { \
	env.stats.start_time = clock(); \
} while (0)

#define END_CLOCK(module) do { \
	env.stats.end_time = clock(); \
    env.stats.times[module] = ((double)(env.stats.end_time - \
        env.stats.start_time)) / CLOCKS_PER_SEC; \
} while (0)

static void opp_debug_info(const struct Opp_Scan* const s, 
						   const struct Stats* const stats)
{
	colored_print(CL_GREEN, "--- Opp Compiler Debug Info ---\n");
	printf("File: %s\n", s->io.fname);
	printf("Size: %ld\n", s->io.fsize);
	printf("Benchmarks:\n");
	printf(" - Parser: %lf\n", stats->times[S_PARSER]);
	printf(" - VM:     %lf\n", stats->times[S_VM]);
	printf("Total:     %lf\n", stats->times[S_PARSER] + stats->times[S_VM]);
	#ifdef X86_64_CPU
	printf("ARCH: x86-64\n");
	#else
	colored_print(CL_RED, "ARCH: ERROR\n");
	#endif

	#if defined(SYSTEM_ERROR)
	colored_print(CL_RED, "OS: ERROR\n");
	#elif defined(SYSTEM_LINUX_64)
	printf("OS: Linux-64\n");
	#elif defined(MAC64)
	printf("OS: Macho-64\n");
	#endif
}

void opp_init_module(struct Opp_State* const state)
{
	struct {
		struct Opp_Scan* scan;
		struct Opp_Parser* parser;
		struct Opp_VM* vm;
		struct Stats stats;
	} env = {0};

	enum Error_State error = ERROR_RECOVER(state->error_buf);

	if (error != OPP_ERROR) {
	
		env.scan = opp_init_lex(state);

		if (!opp_init_file(env.scan, state->fname)) {
			fprintf(stdout, "File not found '%s'...\n", state->fname);
			goto err;
		}

		dump_tokens(env.scan);

		// START_CLOCK();
		// env.parser = opp_init_parser(env.scan);
		// opp_begin_parser(env.parser);
		// END_CLOCK(S_PARSER);

		// START_CLOCK();
		// env.vm = opp_init_vm(env.parser);
		// opp_dump_vm(env.vm);
		// END_CLOCK(S_VM);

	} else {
		fprintf(stdout, "Recovered from error...\n");
	}

	if ((state->flags & F_DEBUG) == F_DEBUG)
		opp_debug_info(env.scan, &env.stats);

err:
	opp_free_lex(env.scan);
	return;
}

#define OPP_VERISION "0.0.1"
static void opp_help()
{
	colored_print(CL_GREEN, "O++ Programming Language\n");
	printf("Version: %s\n", OPP_VERISION);
	printf("\nFormat: opp [flags] [file]\n");
	printf("\t-w              | Hide warnings\n");
	printf("\t-debug          | Debug compiler statistics\n");
	printf("\n");
}

static bool opp_args(const int argc, const char** argv, struct Opp_State* state)
{
	#define CMP(a) !strcmp(argv[i], a)

	for (int i = 1; i < argc - 1; i++) {
		if (CMP("-debug")) {
			state->flags |= F_DEBUG;
		}
		else {
			colored_printf(CL_RED, "Invalid argument '%s'\n", argv[i]);
			return false;
		}
	}

	state->fname = (char*)argv[argc-1];

	return true;
}

static inline void opp_checks()
{
	#ifndef X86_64_CPU
	colored_print(CL_YELLOW, "[ WARNING ] Opp currently only is tested on the x86-64 architecture\n");
	#endif
}

int main(int argc, const char** argv) 
{
	struct Opp_State status = {0};

	opp_checks();

	if (argc < 2) {
		opp_help();
		goto err;
	}

	if (!opp_args(argc, argv, &status))	
		goto err;

	opp_init_module(&status);

err:
	return 0;
}