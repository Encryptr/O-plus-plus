/** @file lexer.h
 * 
 * @brief Opp Lexer
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
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdarg.h>

#ifndef OPP_LEXER
#define OPP_LEXER

#define ignore(i) (i == ' ' || i == '\t' || i=='\r')
#define isletter(i) ((i >= 'a' && i <= 'z') || \
		(i >= 'A' && i <= 'Z') || (i == '_'))
#define isnum(i) ((i >= '0' && i <= '9'))

struct Opp_IO {
	const char* fname;
	FILE* file;
	long fsize;
};

enum Opp_Token {
	INVALID,

	TIDENT, TINTEGER, TFLOATING, TSTR,

	// || && > <= < >= 
	TOR, TAND, TGT, TLE, TLT, TGE,
	// == != ! ?
	TEQEQ, TNOTEQ, TNOT, TQUESTION, 
	// + - / * % -- ++
	TADD, TMIN, TDIV, TMUL, TMOD, TDECR, TINCR,
	// & : ... 
	TADDR, TCOLON, TVA_ARGS,
	// = += -= /= *= %= 
	TEQ, TADDEQ, TMINEQ, TDIVEQ, TMULEQ, TMODEQ,
	// ( ) , [ ] { } . ; ->
	TOPENP, TCLOSEP, TCOMMA, TOPENB, TCLOSEB, 
	TOPENC, TCLOSEC, TDOT, TSEMICOLON, TARROW,
	// << >> ^ | ~
	TSHL, TSHR, TBITXOR, TBITOR, TBITNOT,
    // <<= >>= &= |= ~= ^= 
	TSHLEQ, TSHREQ, TANDEQ, TOREQ, TBNOTEQ, TXOREQ, 
	// Keywords
    TAUTO,
    TBREAK,
    TCASE,
    TCONST,
    TCONTINUE,
    TDEFAULT,
    TDO,
    TWHILE,
    TIF,
    TELSE,
    TEXTERN,
    TFOR,
    TGOTO,
    TREGITSTER,
    TRETURN,
    TSIZEOF,
    TSTATIC,
    TSWITCH,
    TTYPEDEF,
    TVOLATILE,
    TUNSIGNED,
    TSIGNED,

    // Types
    TSTRUCT,
    TENUM,
	TUNION,
    TCHAR,
    TSHORT,
    TINT,
    TLONG,
    TFLOAT,
    TDOUBLE,
	TVOID,
	TINLINE,
	TRESTRICT,
	
	FEND
};

#define SCAN_BUFFER_INITAL 32

struct Opp_Buf {
	char* buf;
	size_t len;
};

struct Opp_Tok {
	enum Opp_Token id;
	struct Opp_Buf buffer;
	union {
		int64_t num;
		double	real;
	} value;
};

struct Opp_Scan {
	char* src, *content;
	struct Opp_IO io;
	uint32_t line, colum;
	struct Opp_Tok t;
	char* peek;
};

// Lexer
void dump_tokens(struct Opp_Scan* s);
struct Opp_Scan* opp_init_lex(const char* fname);
void opp_next(struct Opp_Scan* s);
enum Opp_Token opp_peek_tok(struct Opp_Scan* s, int times);
const char* tok_to_str(struct Opp_Scan* s);
const char* tok_debug(enum Opp_Token tok);

#endif /* OPP_LEXER */
