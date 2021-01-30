/** @file lexer.h
 * 
 * @brief Lexical analysis
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

#ifndef OPP_LEXER
#define OPP_LEXER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include "../header/os.h"
#include "io.h"

enum Opp_Token {
	INVALID,

	TIDENT, TINTEGER, TFLOAT, THEX, TSTR, TCH,

	// || && > < >= <= 
	TOR, TAND, TGT, TLE, TLT, TGE,

	// == = ! != ' "
	TEQEQ, TNOTEQ, TEQ, TNOT, TTIK, TQUOTE,	
	
	// + - / * % -- ++
	TADD, TMIN, TDIV, TMUL, TMOD, TDECR, TINCR,

	// & : ...
	TADDR, TCOLON, TVA_ARGS,

	// += -= /= *= %= 
	TADDEQ, TMINEQ, TDIVEQ, TMULEQ, TMODEQ,

	// ( ) , [ ] . ; 
	TOPENP, TCLOSEP, TCOMMA, TOPENB, TCLOSEB, 
	TOPENC, TCLOSEC, TDOT, TSEMICOLON,

	// << >> ^ | ~
	TSHL, TSHR, TBITXOR, TBITOR, TBITNOT,

	// O++ Keywords
	TASM, TAUTO, TSIZEOF, TWHILE, TIF, 
	TELSE, TRET, TSWITCH, TSTRUCT, TGOTO,
	TEXTERN, TIMPORT, TFOR, TCASE, TBREAK,
	TUNSIGNED,
	
	FEND
};

#define SCAN_BUF 32
#define INCR s->src++; s->colum++
#define DECR s->src--; s->colum--

struct Opp_Buf {
	char* buf;
	size_t len;
};

typedef struct Opp_Buf OppBuf;

struct Opp_Tok {
	enum Opp_Token id;
	OppBuf buffer;
	union {
		int64_t num;
		double	real;
	};
};

struct Opp_Scan {
	char* src;
	OppIO io;
	uint32_t line;
	uint32_t colum;
	struct Opp_Tok t;
};

#define INTERNAL_ERROR(str) \
	internal_error(str, __FUNCTION__);

struct Opp_Scan;

void opp_error(struct Opp_Scan* s, const char* str, ...);
void internal_error(const char* str, const char* func);

uint64_t hex2i64(char *str);
bool is_keyword(struct Opp_Scan* s);
void dump_tokens(struct Opp_Scan* s);

// OppLex
void init_opp_file(struct Opp_Scan* s, const char* fname);
void init_opp_lex(struct Opp_Scan* s, char* content);
void opp_next(struct Opp_Scan* s);
void opp_deinit(struct Opp_Scan* s);
void opp_peek_tok(struct Opp_Scan* s, int times);
void opp_lex_identifier(struct Opp_Scan* s);
void opp_lex_numeral(struct Opp_Scan* s);
void opp_lex_hex(struct Opp_Scan* s);
void opp_lex_str(struct Opp_Scan* s);
enum Opp_Token opp_lex_char(struct Opp_Scan* s);
void realloc_buf(struct Opp_Scan* s);


#endif /* OPP_LEXER */