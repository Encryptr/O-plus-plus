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
#include <time.h>
#include <unistd.h>
#include "opp.h"

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
#define GET_TIME(elem) \
	stats.list[elem] = ((double) (stats.end - stats.start)) / CLOCKS_PER_SEC

// static void opp_debug_info(Opp_Stats* s, struct Opp_Context* opp)
// {
// 	printf("%s### Opp Compiler Debug Info ###\n%s", CL_GREEN, CL_RESET);
// 	printf("File: %s\n", opp->parser->lex->io.fname);
// 	printf("Size: %ld\n", opp->parser->lex->io.fsize);
// 	printf("Benchmarks:\n");
// 	printf(" - Parser:   %lf\n", s->list[S_PARSER]);
// 	printf(" - Analizer: %lf\n", s->list[S_ANALIZE]);
// 	printf(" - Compiler: %lf\n", s->list[S_COMPILER]);
// 	printf(" - Ir:       %lf\n", s->list[S_IR]);
// 	printf("Total: %lf\n", s->list[S_PARSER] + s->list[S_ANALIZE] 
// 		+ s->list[S_COMPILER] + s->list[S_IR]);
// 	#ifdef X86_CPU
// 	printf("ARCH: x86-64\n");
// 	#else
// 	printf("ARCH: ERROR\n");
// 	#endif
// 	#ifdef OS_ERROR
// 	printf("OS: ERROR\n");
// 	#elif defined(LINUX64)
// 	printf("OS: Linux64\n");
// 	#elif defined(MAC64)
// 	printf("OS: Macos64\n");
// 	#endif
// 	printf("%s################################%s\n", CL_GREEN, CL_RESET);
// }

// void opp_init_module(const char* fname, struct Opp_Options* opts)
// {
// 	Opp_Stats stats = {0};
// 	struct Opp_Scan scan = {0};

// 	opp_init_file(fname, &scan);

// 	START_CLOCK();
// 	struct Opp_Parser* parser = opp_parser_init(&scan);
// 	opp_parser_begin(parser);
// 	END_CLOCK();
// 	GET_TIME(S_PARSER);

// 	START_CLOCK();
// 	struct Opp_Analize* ctx = opp_init_analize(parser, opts);
// 	analize_tree(ctx);
// 	END_CLOCK();
// 	GET_TIME(S_ANALIZE);

// 	START_CLOCK();
// 	struct Opp_Context* context = opp_init_compile(parser, opts);
// 	opp_compile(context);
// 	END_CLOCK();
// 	GET_TIME(S_COMPILER);

// 	START_CLOCK();
// 	context->oppir = init_oppir();
// 	oppir_get_opcodes(context->oppir, &context->ir);
// 	oppir_eval(context->oppir);
// 	OppIO io = { .file = fopen("out.o", "wb") };
// 	oppir_emit_obj(context->oppir, &io);
// 	END_CLOCK();
// 	GET_TIME(S_IR);
// 	fclose(io.file);


// 	if (opts->run_output) 
// 		system("gcc ./stdlib/testing.o out.o");

// 	if (opts->debug)
// 		opp_debug_info(&stats, context);

// 	oppir_free(context->oppir);
// 	opp_free_compiler(context);
// 	opp_free_analize(ctx);
// 	opp_free_parser(parser);
// 	opp_free_lex(&scan, false);
// }

// void opp_add_module(struct Opp_Parser* old, char* fname)
// {
// 	struct Opp_Scan* new_scan = (struct Opp_Scan*)malloc(sizeof(struct Opp_Scan));

// 	if (new_scan == NULL)
// 		INTERNAL_ERROR("Malloc fail");

// 	char* path = fname;
// 	if (access(path, F_OK) == -1) {
// 		if (stdheader_path == NULL)
// 			INTERNAL_ERROR("Unknown path to stdlib");
// 		path = (char*)malloc(strlen(fname) + strlen(stdheader_path) + 1);
// 		sprintf(path, "%s%s", stdheader_path, fname);
// 	}

// 	opp_init_file(path, new_scan);

// 	struct Opp_Scan* temp = old->lex;
// 	old->lex = new_scan;

// 	opp_parser_begin(old);

// 	old->lex = temp;
	
// 	opp_free_lex(new_scan, 1);
// 	free(path);
// }

// #include <sys/stat.h>
// static void opp_live_compiler(struct Opp_Options* opts, const char* fname)
// {
// 	printf("%s### Opp Live Compiler ###%s\n", CL_GREEN, CL_RESET);
// 	unsigned int amount = 0;

// 	struct Opp_Scan* scan = (struct Opp_Scan*)malloc(sizeof(struct Opp_Scan));
// 	opp_init_file(fname, scan);

// 	struct Opp_Parser* parser = opp_parser_init(scan);
// 	opp_parser_begin(parser);

// 	struct Opp_Analize* ctx = opp_init_analize(parser, opts);
// 	analize_tree(ctx);

// 	struct stat filestat;
// 	stat(fname, &filestat);
// 	time_t past = filestat.st_mtime;
// 	time_t current;


// 	for (;;) {
// 		stat(fname, &filestat);
// 		current = filestat.st_mtime;

// 		if (past < current) {
// 			printf(">> %sOpp file '%s' has been modified [%u]%s\n", CL_BLUE, fname, amount++, CL_RESET);
			
// 			// allocator_reset();

// 			// opp_free_analize(ctx);
// 			// opp_free_parser(parser);
// 			// opp_free_lex(scan, true);

// 			opp_init_file(fname, scan);

// 			// parser = opp_parser_init(scan);
// 			// opp_parser_begin(parser);

// 			// ctx = opp_init_analize(parser, opts);
// 			// analize_tree(ctx);

// 			past = current;
// 		}
// 	}

// }

// #define CMP(a) !strcmp(argv[i], a)
// #define OPP_VERISION "0.0.1"

// static void opp_help()
// {
// 	printf("\x1b[32mO++ Programming Language\x1b[0m\n");
// 	printf("\x1b[32mVersion: %s\x1b[0m\n", OPP_VERISION);
// 	printf("\n\x1b[31mFormat: opp [flags] [file]\x1b[0m\n");
// 	printf("\trun             | Generate executable and run\n");
// 	printf("\tbuild (default) | Generate obj file from source\n");
// 	printf("\ttest            | Only analize file do not compile\n");
// 	printf("\tlive            | Live compilation\n");
// 	printf("\t-d              | For dumping file tokens\n");
// 	printf("\t-lc             | Link C's standard library\n");
// 	printf("\t-nostd          | Don't link Opp stdlib\n");
// 	printf("\t-w              | Hide warnings\n");
// 	printf("\t-debug          | Debug compiler statistics\n");
// 	printf("\t-Wall           | Turn warnings to errors\n");
// 	printf("\t-path [path]    | Specify std headers path\n");
// 	printf("\n");
// }

// static void opp_args(int argc, const char** argv, struct Opp_Options* opts)
// {
// 	// Defaults
// 	opts->warning = true;
// 	stdheader_path = "./stdlib/";
// 	for (int i = 1; i < argc - 1; i++) {
// 		if (CMP("run")) opts->run_output = true;
// 		else if (CMP("build")) opts->run_output = false;
// 		else if (CMP("test")) opts->test = true;
// 		else if (CMP("live")) opts->live = true;
// 		else if (CMP("-d")) opts->dump_toks  = true;
// 		else if (CMP("-lc")) opts->link_c    = true;
// 		else if (CMP("-nostd")) opts->nostd  = true;
// 		else if (CMP("-w")) opts->warning    = false;
// 		else if (CMP("-debug")) opts->debug  = true;
// 		else if (CMP("-Wall")) opts->wall    = true;
// 		else if (CMP("-path")) stdheader_path = argv[++i];
// 		else printf("%sInvalid argument '%s'%s\n", CL_RED, argv[i], CL_RESET);
// 	}
// }

// static bool opp_init()
// {
// 	#ifndef X86_CPU
// 	printf("%sOpp currently only supports the x86-64 architecture%s\n",
// 		CL_RED, CL_RESET);
// 	return false;
// 	#endif

// 	#ifdef OS_ERROR
// 	printf("%sOpp currently only supports:\n\t- Linux (64 bit)%s\n",
// 		CL_RED, CL_RESET);
// 	return false;
// 	#endif

// 	return true;
// }

int main(int argc, const char** argv) 
{
	// struct Opp_Options opts = {0};

	// if (!opp_init())
	// 	exit(1);

	// if (!allocator_init())
	// 	INTERNAL_ERROR("Allocator init fail");

	// if (argc < 2) {
	// 	opp_help();
	// 	return 0;
	// }

	// opp_args(argc, argv, &opts);
	
	// if (opts.live)
	// 	opp_live_compiler(&opts, argv[argc-1]);
	// else
	// 	opp_init_module(argv[argc-1], &opts);

	// // objdump -b binary -D -m i386:x86-64 -M intel out.bin 

	// allocator_free();

	return 0;
}