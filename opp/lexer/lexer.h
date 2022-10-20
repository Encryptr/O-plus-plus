/** @file lexer.h
 * 
 * @brief Opp Lexer
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
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdarg.h>
#include "../opp.h"

#ifndef OPP_LEXER
#define OPP_LEXER

struct Opp_IO {
	const char* fname;
	FILE* file;
	long fsize;
};

enum Opp_Token {
	INVALID,

	TIDENT, TINTEGER, TFLOATING, TSTR,

	// Characters
	TK_CHARS,
	// =
	TEQ, 
	// > <= < >= 
	TGT, TLE, TLT, TGE,
	// == != ! 
	TEQEQ, TNOTEQ, TNOT,  
	// + - / * % 
	TADD, TMIN, TDIV, TMUL, TMOD,
	// << >> ^ | ~ &
	TSHL, TSHR, TBITXOR, TBITOR, TBITNOT, TADDR,
	// ... -- ++
	TVA_ARGS, TDECR, TINCR,
	// ( ) , [ ] { } . ; ->
	TOPENP, TCLOSEP, TCOMMA, TOPENB, TCLOSEB, 
	TOPENC, TCLOSEC, TDOT, TSEMICOLON, TARROW,

	// Keywords
	TK_KEYWORDS,

	TOR, TAND, TAUTO,
	TFUNC, TIF, TELIF, 
	TELSE, TWHILE, TFOR,

	FEND
};

struct Opp_Tok {
	enum Opp_Token id;
	char* buffer;
	unsigned int size;
	union {
		int64_t num;
		double	real;
	} value;
};

struct Opp_Scan {
	struct Opp_State* state;
	char* src, *content;
	struct Opp_IO io;
	unsigned int line, colum;
	struct Opp_Tok t, peek;
};

void dump_tokens(struct Opp_Scan* s);
struct Opp_Scan* opp_init_lex(struct Opp_State* state);
void opp_free_lex(struct Opp_Scan* s);
bool opp_init_file(struct Opp_Scan* s, const char* fname);
void opp_init_from_buffer(struct Opp_Scan* s, char* const buffer);
void opp_next(struct Opp_Scan* s);
enum Opp_Token opp_peek_tok(struct Opp_Scan* s, unsigned int times);
const char* tok_to_str(struct Opp_Scan* s);
const char* tok_debug(enum Opp_Token tok);
void opp_error(struct Opp_Scan* s, const char* str, ...);

#endif /* OPP_LEXER */
