/** @file lexer.c
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

#include "lexer.h"
#include "../util/util.h"
#include "../memory/memory.h"

static void opp_lex_identifier(struct Opp_Scan* s);
static void opp_lex_hex(struct Opp_Scan* s);
static void opp_lex_numeral(struct Opp_Scan* s);
static void opp_lex_octal(struct Opp_Scan* s);
static void opp_lex_str(struct Opp_Scan* s);
static void opp_lex_tik(struct Opp_Scan* s);
static enum Opp_Token opp_lex_char(struct Opp_Scan* s);
static void opp_lex_escape(struct Opp_Scan* s, unsigned int idx);

#define INCR { s->src++; s->colum++; }
#define DECR { s->src--; s->colum--; }
#define CHECK_BUFFER() \
	if (idx == s->t.buffer.len) \
		realloc_buf(s);

static uint64_t hex2i64(char *str)
{
	uint64_t res = 0;
	char c;

	while ((c = *str++)) {
        char v = ((c & 0xF) + (c >> 6)) | ((c >> 3) & 0x8);
		res = (res << 4) | (uint64_t) v;
	}

	return res;
}

struct Opp_Str_Pair {
	const char* key;
	enum Opp_Token id;
};

#define KEYWORD_AMOUNT sizeof(opp_keys)/sizeof(opp_keys[0])
static const struct Opp_Str_Pair opp_keys[] = {
	{0,0}, {0,0}, {0,0}, {0,0}, {0,0},

	// Characters
	{"||", TOR},     {"&&", TAND},      {">", TGT},        {"<=", TLE},
	{"<", TLT},      {">=", TGE},       {"==", TEQEQ},     {"!=", TNOTEQ},
	{"!", TNOT},     {"?", TQUESTION},  {"+", TADD},       {"-", TMIN},
	{"/", TDIV},     {"*", TMUL},       {"%%", TMOD},      {"--", TDECR},     
	{"++", TINCR},   {"&", TADDR},      {":", TCOLON},     {"...", TVA_ARGS}, 
	{"=", TEQ},    
	{"+=", TADDEQ},  {"-=", TMINEQ},    {"/=", TDIVEQ},    {"*=", TMULEQ}, 
	{"%%=", TMODEQ}, {"(", TOPENP},     {")", TCLOSEP},    {",", TCOMMA},  
	{"[", TOPENB},   {"]", TCLOSEB},    {"{", TOPENC},     {"}", TCLOSEC}, 
	{".", TDOT},     {";", TSEMICOLON}, {"->", TARROW},    {"<<", TSHL},      
	{">>", TSHR},    {"^", TBITXOR},    {"|", TBITOR},     {"~", TBITNOT},    
	{"<<=", TSHLEQ}, {">>=", TSHREQ},   {"&=", TANDEQ},    {"|=", TOREQ},     
	{"~=", TBNOTEQ}, {"^=", TXOREQ},

	// Keywords 
	{"auto", TAUTO},          {"break", TBREAK},        {"case", TCASE},         
	{"const", TCONST},        {"continue", TCONTINUE},  {"default", TDEFAULT}, 
	{"do", TDO},              {"while", TWHILE},        {"if", TIF},              
	{"else", TELSE},          {"extern", TEXTERN},      {"for", TFOR}, 
	{"goto", TGOTO},          {"register", TREGITSTER}, {"return", TRETURN},   
	{"sizeof", TSIZEOF},      {"static", TSTATIC},      {"switch", TSWITCH},      
	{"typedef", TTYPEDEF},    {"volatile", TVOLATILE},  {"unsigned", TUNSIGNED},
	{"signed", TSIGNED},      {"struct", TSTRUCT},      {"enum", TENUM},
	{"union", TUNION},        {"char", TCHAR},          {"short", TSHORT},
	{"int", TINT},            {"long", TLONG},          {"float", TFLOAT},
	{"double", TDOUBLE},      {"void", TVOID},          {"inline", TINLINE},
	{"restrict", TRESTRICT},
	
	{0,0}
};

static bool is_keyword(struct Opp_Scan* s)
{
	for (int i = TAUTO; i < FEND; i++) {
		if (!strcmp(opp_keys[i].key, s->t.buffer.buf)) {
			s->t.id = opp_keys[i].id;
			return true;
		}
	}
	return false;
}

static void realloc_buf(struct Opp_Scan* s)
{
	char* temp = opp_realloc(s->t.buffer.buf, 
		s->t.buffer.len+32, s->t.buffer.len);
	if (!temp)
		MALLOC_FAIL();
	s->t.buffer.len += 32;
	s->t.buffer.buf = temp;
}

void opp_init_file(struct Opp_Scan* s, const char* fname)
{
	long size = 0;
	s->io.fname = fname;
	s->io.file = fopen(fname, "r");

	if (!s->io.file) {
		printf("Opp error opening file '%s'\n", fname);
		exit(1);
	}

	fseek(s->io.file, 0, SEEK_END);
	size = ftell(s->io.file);
	s->io.fsize = size;
	rewind(s->io.file);
	s->content = opp_alloc(size + 1);

	if (!s->content)
		MALLOC_FAIL();
	memset(s->content, 0, size + 1);

	(void)fread(s->content, size, 1, s->io.file);
	fclose(s->io.file);
	s->src = s->content;
}

struct Opp_Scan* opp_init_lex(const char* fname)
{
	struct Opp_Scan* s = (struct Opp_Scan*)
		opp_alloc(sizeof(struct Opp_Scan));

	if (!s)
		MALLOC_FAIL();

	opp_init_file(s, fname);
	s->line = 1;
	s->colum = 0;
	s->t.buffer.buf = (char*)opp_alloc(SCAN_BUFFER_INITAL);
	if (!s->t.buffer.buf)
		MALLOC_FAIL();
	s->t.buffer.len = SCAN_BUFFER_INITAL;

	return s;
}

const char* tok_to_str(struct Opp_Scan* s)
{
	switch (s->t.id)
	{
		case TIDENT:
		case TSTR:
		case TINTEGER:
		case TFLOATING:
			return s->t.buffer.buf;

		default:
			return opp_keys[s->t.id].key;
	}
}

const char* tok_debug(enum Opp_Token tok)
{
	return opp_keys[(int)tok].key;
}

static void opp_lex_identifier(struct Opp_Scan* s)
{
	unsigned int idx = 0;
	while (isletter(*s->src) || isnum(*s->src)) {
		CHECK_BUFFER();
		s->t.buffer.buf[idx] = *s->src;
		idx++;
		INCR;
	}
	CHECK_BUFFER();
	s->t.buffer.buf[idx] = '\0';
	if (is_keyword(s))
		return;
	s->t.id = TIDENT;
}

static void opp_lex_hex(struct Opp_Scan* s)
{
	unsigned int idx = 0;
	s->t.id = TINTEGER;
	INCR;
	while (isletter(*s->src) || isnum(*s->src)) {
		CHECK_BUFFER();
		s->t.buffer.buf[idx] = *s->src;
		idx++;
		INCR;
	}
	CHECK_BUFFER();
	s->t.buffer.buf[idx] = '\0';
	s->t.value.num = hex2i64(s->t.buffer.buf);

	if (*s->src == 'u' || *s->src == 'U' ||
		*s->src == 'l' || *s->src == 'L' ||
		*s->src == 'F' || *s->src == 'f') {
		INCR;
	}
}

static void opp_lex_octal(struct Opp_Scan* s)
{
	unsigned int idx = 0;
	while (isnum(*s->src)) {
		CHECK_BUFFER();
		s->t.buffer.buf[idx] = *s->src;
		idx++;
		INCR;
	}
	CHECK_BUFFER();
	s->t.buffer.buf[idx] = '\0';

	s->t.value.num = strtol(s->t.buffer.buf, NULL, 8);

	if (*s->src == 'u' || *s->src == 'U' ||
		*s->src == 'l' || *s->src == 'L') {
		INCR;
	}
	else if (*s->src == 'F' || *s->src == 'f') {
		opp_error(s, "Invalid suffix to octal '%s'", s->t.buffer.buf);
	}
}

static void opp_lex_numeral(struct Opp_Scan* s)
{
	unsigned int idx = 0;
	s->t.id = TINTEGER;
	INCR;
	if (*s->src == 'x' || *s->src == 'X') {
		opp_lex_hex(s);
		return;
	}
	else {
		DECR;
		if (*s->src == '0' && isnum(s->src[1])) {
			opp_lex_octal(s);
			return;
		}
	}
	
	while (isnum(*s->src) || *s->src == '.') {
		if (*s->src == '.') 
			s->t.id = TFLOAT;
		CHECK_BUFFER();

		s->t.buffer.buf[idx] = *s->src;
		idx++;
		INCR;
	}
	CHECK_BUFFER();
	s->t.buffer.buf[idx] = '\0';

	if (*s->src == 'u' || *s->src == 'U' ||
		*s->src == 'l' || *s->src == 'L' ||
		*s->src == 'F' || *s->src == 'f')
		INCR;

	switch (s->t.id)
	{
		case TINTEGER: 
			s->t.value.num = atol(s->t.buffer.buf);
			break;

		case TFLOAT:
			s->t.value.real = atof(s->t.buffer.buf);
			break;

		default: break;
	}
}

static void opp_lex_escape(struct Opp_Scan* s, unsigned int idx)
{
	switch (*s->src) {
		case 'n': 
			s->t.buffer.buf[idx] = '\n';
			break;

		case 't':
			s->t.buffer.buf[idx] = '\t';
			break;

		case '\\':
			s->t.buffer.buf[idx] = '\\';
			break;

		case '\'':
			s->t.buffer.buf[idx] = '\'';
			break;

		case '"':
			s->t.buffer.buf[idx] = '\"';
			break;

		case 'v':
			s->t.buffer.buf[idx] = '\v';
			break;
		
		case 'b':
			s->t.buffer.buf[idx] = '\b';
			break;

		case 'r':
			s->t.buffer.buf[idx] = '\r';
			break;

		case 'f':
			s->t.buffer.buf[idx] = '\f';
			break;
		
		case 'a':
			s->t.buffer.buf[idx] = '\a';
			break;
		
		case '?':
			s->t.buffer.buf[idx] = '\?';
			break;

		case '0':
			s->t.buffer.buf[idx] = '\0';
			break;

		default:
			opp_error(s, "Invalid escape character '%c'", *s->src);
	}
}

static void opp_lex_str(struct Opp_Scan* s)
{
	if (*s->src == 'L') {
		INCR;
	}

	INCR;
	unsigned int idx = 0;
	while (*s->src != '"') {
		CHECK_BUFFER();
		if (!(*s->src) || *s->src == '\n')
			opp_error(s, "Expected terminating '\"' in string");

		else if (*s->src == '\\') {
			INCR;
			opp_lex_escape(s, idx);
		}
		else
			s->t.buffer.buf[idx] = *s->src;
		idx++;
		INCR;
	}
	INCR;
	CHECK_BUFFER();
	s->t.buffer.buf[idx] = '\0';
	s->t.id = TSTR;
}

static void opp_lex_tik(struct Opp_Scan* s)
{
	if (*s->src == 'L') {
		INCR;
	}

	INCR;
	union {
		char c[4];
		int i;
	} value = {0};

	unsigned int idx = 0;
	while (*s->src != '\'') {
		if (*s->src == '\n' || *s->src == 0)
			opp_error(s, "Terminating ' not found");
		if (idx == 4)
			opp_error(s, "\' Contains more then 4 characters");
		if (*s->src == '\\') {
			INCR;
			opp_lex_escape(s, 0);
			value.c[idx] = s->t.buffer.buf[0];
		}
		else
			value.c[idx] = *s->src;
		idx++;
		INCR;
	}

	s->t.value.num = value.i;
	s->t.id = TINTEGER;
	INCR;
}

#define CHECK(c) *s->src == c
static enum Opp_Token opp_lex_char(struct Opp_Scan* s)
{
	switch (*s->src)
	{
		case ':': return TCOLON;
		case ';': return TSEMICOLON;
		case '(': return TOPENP;
		case ')': return TCLOSEP;
		case '{': return TOPENC;
		case '}': return TCLOSEC;
		case '[': return TOPENB;
		case ']': return TCLOSEB;
		case ',': return TCOMMA;
		
		case '.': {
			INCR;
			if (CHECK('.')) {
				INCR;
				if (CHECK('.'))	return TVA_ARGS;
				break;
			}
			DECR;
			return TDOT;
		}

		case '|': {
			INCR;
			if (CHECK('|')) return TOR;
			else if (CHECK('=')) return TOREQ;
			DECR;
			return TOR;
		}

		case '&': {
			INCR;
			if (CHECK('&')) return TAND;
			else if (CHECK('=')) return TANDEQ;
			DECR;
			return TADDR;
		}

		case '~': {
			INCR;
			if (CHECK('=')) return TBNOTEQ;
			DECR;
			return TBITNOT;
		}

		case '^': {
			INCR;
			if (CHECK('=')) return TXOREQ;
			DECR;
			return TBITXOR;
		}

		case '>': {
			INCR;
			if (CHECK('=')) return TGE;
			else if (CHECK('>')) {
				INCR;
				if (CHECK('=')) return TSHREQ;
				DECR;
				return TSHR;
			}
			DECR;
			return TGT;
		}

		case '<': {
			INCR;
			if (CHECK('=')) return TLE;
			else if (CHECK('<')) {
				INCR;
				if (CHECK('=')) return TSHLEQ;
				DECR;
				return TSHL;
			}
			DECR;
			return TLT;
		}

		case '=': {
			INCR;
			if (CHECK('=')) return TEQEQ;
			DECR;
			return TEQ;
		}

		case '!': {
			INCR;
			if (CHECK('=')) return TNOTEQ;
			DECR;
			return TNOT;
		}

		case '+': {
			INCR;
			if (CHECK('=')) return TADDEQ;
			else if (CHECK('+')) return TINCR;
			DECR;
			return TADD;
		}

		case '-': {
			INCR;
			if (CHECK('=')) return TMINEQ;
			else if (CHECK('-')) return TDECR;
			else if (CHECK('>')) return TARROW;
			DECR;
			return TMIN;
		}

		case '*': {
			INCR;
			if (CHECK('=')) return TMULEQ;
			DECR;
			return TMUL;
		}

		case '/': {
			INCR;
			if (CHECK('=')) return TDIVEQ;
			DECR;
			return TDIV;
		}

		case '%': {
			INCR;
			if (CHECK('=')) return TMODEQ;
			DECR;
			return TMOD;
		}
		default: break;
	}
	return INVALID;
}

void opp_peek_tok(struct Opp_Scan* s, int times)
{
	char* temp = s->src;
	uint32_t tline = s->line;
	uint32_t tcolum = s->colum;

	for (int i = 0; i < times; i++)
		opp_next(s);

	s->src = temp;
	s->line = tline;
	s->colum = tcolum;
}

void opp_next(struct Opp_Scan* s)
{
	memset(s->t.buffer.buf, 0, s->t.buffer.len);
	while (*s->src)
	{
		if (*s->src == '\n') {
			s->line++;
			s->colum = 0;
		}
		else if (ignore(*s->src)) {}
			/////////
		else if (*s->src == '/' && s->src[1] == '/') {
			//delete this
			while (*s->src && *s->src != '\n') {
				INCR;
			}
			s->line++;
			s->colum = 0;
		}
		////////
		else if (*s->src == '\"' || (*s->src == 'L' 
				&& s->src[1] == '\"')) {
			opp_lex_str(s);
			return;
		}
		else if (*s->src == '\'' || (*s->src == 'L' 
				&& s->src[1] == '\'')) {
			opp_lex_tik(s);
			return;
		}
		else if (isletter(*s->src)) {
			opp_lex_identifier(s); 
			return;
		}
		else if (isnum(*s->src)) {
			opp_lex_numeral(s);
			return;
		}
		else {
			s->t.id = opp_lex_char(s);
			if (s->t.id == INVALID) 
				opp_error(s, "Invalid token found");
			INCR;
			return;
		}
		INCR;
	}
	s->t.id = FEND;
}

void dump_tokens(struct Opp_Scan* s)
{
	do {
		opp_next(s);
		printf("%s:%d:%d\t", s->io.fname, s->line, s->colum);
		if (s->t.id == TFLOAT)
			printf("FLOAT\t %lf\n", s->t.value.real);
		else if (s->t.id == TINTEGER)
			printf("INT\t %lld\n", s->t.value.num);
		else if (s->t.id == TIDENT)
			printf("IDENT\t %s\n", s->t.buffer.buf);
		else if (s->t.id == TSTR)
			printf("STRING\t \"%s\"\n", s->t.buffer.buf);
		else if (s->t.id >= TOR && s->t.id <= TXOREQ)
			printf("CHAR\t '%s'\n", opp_keys[s->t.id].key);
		else if (s->t.id >= TAUTO && s->t.id <= TINLINE)
			printf("KEYWORD\t '%s'\n", opp_keys[s->t.id].key);
		else if (s->t.id == FEND)
			printf("FILE END\n");

	} while (s->t.id != FEND);
}