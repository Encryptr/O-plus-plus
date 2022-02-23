/** @file lexer.c
 * 
 * @brief Opp Lexer
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

#include "lexer.h"
#include "../util/util.h"
#include "../memory/memory.h"
#include "../util/platform.h"

static void opp_lex_identifier(struct Opp_Scan* s);
static void opp_lex_hex(struct Opp_Scan* s);
static void opp_lex_numeral(struct Opp_Scan* s);
static void opp_lex_str(struct Opp_Scan* s);
static void opp_lex_tik(struct Opp_Scan* s);
static enum Opp_Token opp_lex_char(struct Opp_Scan* s);
static char opp_lex_escape(struct Opp_Scan* s);
static void realloc_buf(struct Opp_Scan* s);

static inline void incr(register struct Opp_Scan* s) 
{
	s->src++;
	s->colum++;
}

static inline void decr(register struct Opp_Scan* s) 
{
	s->src--;
	s->colum--;
}

static inline void write_into_buf(register struct Opp_Scan* s,
								  register int idx,
								  register char c) 
{
	if (idx >= s->t.size) {
		printf("Realloc\n");
		realloc_buf(s);
	}

	s->t.buffer[idx] = c;
}

#define WHITESPACE(i) (i == ' ' || i == '\t' || i == '\r')
#define LETTER(i) ((i >= 'a' && i <= 'z') || \
					(i >= 'A' && i <= 'Z')  || (i == '_'))
#define NUMBER(i) ((i >= '0' && i <= '9'))

// make tarce back lengths args in Opp_Status

static void trace_back(struct Opp_Scan* s)
{
	/* Implement to trace back 1-2 lines on code to show whats around 
		Example (file.opp):
			if 123.3. > 23.5 {
				...
			}

		file.opp:1:5 error: Repeated '.' in number
			if 123.3. > 23.5 {
			   ~~~^~

		Make squigle lines change color

		Trace back will attempt to collect the whole line until 
		\n unless it exceeds its max characters limit
	*/

	#define IN_BOUNDS(ptr) ((ptr > s->content) && (*ptr != 0))

	const int max_characters = 32;
	unsigned int pos = s->src - s->content;
	const unsigned int error_pos = pos;

	for (
		unsigned int i = max_characters+1;
		(i != 0) && (pos > 0) && (s->content[pos-1] != '\n');
		i--
	)
		pos--;

	unsigned int start_pos = pos;

	// colored_print(CL_YELLOW, "... ");

	for (
		unsigned int i = 0;
		(i < max_characters*2) && (pos < s->io.fsize) && (s->content[pos] != '\n');
		i++, pos++
	) {
		fprintf(stdout, "%c", s->content[pos]);
	}
	colored_print(CL_YELLOW, "\n\t");

	for (; start_pos != error_pos; start_pos++)
		colored_print(CL_RED, "~");
	colored_print(CL_BLUE, "^");
	fprintf(stdout, "\n");
}

void opp_error(struct Opp_Scan* s, const char* str, ...)
{
	va_list ap;
	va_start(ap, str);
	
	printf("%s:%u:%u ", s->io.fname, s->line, s->colum);
	
	colored_print(CL_RED, "error: ");
	vfprintf(stdout, str, ap);
	fprintf(stdout, "\n\t");
	va_end(ap);

	trace_back(s);

	THROW_ERROR(s->state->error_buf);
}

static uint64_t hex2i64(char* str)
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

#define KEYWORD_AMOUNT (sizeof(opp_keys) / sizeof(opp_keys[0]))
#define TOKEN(tok, ch) [tok] = {ch, tok}

static const struct Opp_Str_Pair opp_keys[] = {
	TOKEN(INVALID  , 0),  TOKEN(TIDENT    , 0),  
	TOKEN(TINTEGER , 0),  TOKEN(TFLOATING , 0),
	TOKEN(TSTR     , 0),  TOKEN(FEND      , 0),
	
	// Characters
	TOKEN(TGT, ">"),  TOKEN(TLE    , "<="),  TOKEN(TLT, "<"),  
	TOKEN(TGE, ">="),  TOKEN(TEQEQ , "==" ),  TOKEN(TNOTEQ , "!="),  
	TOKEN(TNOT, "!"), TOKEN(TADD, "+"), TOKEN(TMIN, "-"), 
	TOKEN(TDIV, "/"), TOKEN(TMUL, "*"), TOKEN(TMOD, "%%"),
	TOKEN(TDECR, "--"), TOKEN(TINCR, "++"), TOKEN(TVA_ARGS, "..."),


	// Keywords 
};

static bool is_keyword(struct Opp_Scan* s)
{
	for (int i = TOR; i < FEND; i++) {
		if (!strcmp(opp_keys[i].key, s->t.buffer)) {
			s->t.id = opp_keys[i].id;
			return true;
		}
	}
	return false;
}

static void realloc_buf(struct Opp_Scan* s)
{
	char* const temp = opp_realloc(
			s->t.buffer, 
			s->t.size+SCAN_BUFFER_INITAL, 
			s->t.size);

	if (!temp)
		MALLOC_FAIL();

	s->t.size += SCAN_BUFFER_INITAL;
	s->t.buffer = temp;
}

bool opp_init_file(struct Opp_Scan* s, const char* fname)
{
	long size = 0;
	s->io.fname = fname;
	s->io.file = fopen(fname, "r");

	if (!s->io.file)
		return false;

	fseek(s->io.file, 0, SEEK_END);
	size = ftell(s->io.file); // make sure whole size + 1 stuff works right
	s->io.fsize = size;
	rewind(s->io.file);
	s->content = opp_alloc(size + 1);

	if (!s->content)
		MALLOC_FAIL();
	memset(s->content, 0, size + 1);

	(void)fread(s->content, size, 1, s->io.file);
	fclose(s->io.file);
	s->src = s->content;

	return true;
}

void opp_init_from_buffer(struct Opp_Scan* s, char* const buffer)
{
	s->io.fname = "input";
	s->io.fsize = strlen(buffer);
	s->content = buffer;
	s->src = s->content;
}

struct Opp_Scan* opp_init_lex(struct Opp_State* state)
{
	struct Opp_Scan* s = (struct Opp_Scan*)
		opp_alloc(sizeof(struct Opp_Scan));

	if (!s)
		MALLOC_FAIL();

	s->state = state;
	s->line = 1;
	s->colum = 0;

	s->t.buffer = (char*)opp_alloc(SCAN_BUFFER_INITAL);

	if (!s->t.buffer)
		MALLOC_FAIL();

	memset(s->t.buffer, 0, SCAN_BUFFER_INITAL);
	s->t.size = SCAN_BUFFER_INITAL;

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
			return s->t.buffer;

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
	/* Max identifier length is 32 characters,
	   no check needed for buffer size,
	   it should always hold at least 32 bytes
	*/
	unsigned int idx = 0;

	while (LETTER(*s->src) || 
		   NUMBER(*s->src)) 
	{
		if (idx == 32)
			opp_error(s, "Identifiers can only be 32 characters long");
		s->t.buffer[idx] = *s->src;
		idx++;
		incr(s);
	}
	s->t.buffer[idx] = '\0';

	// if (!is_keyword(s))
		s->t.id = TIDENT;
}

static void opp_lex_hex(struct Opp_Scan* s)
{
	/* Max hex length is 18 bytes
		0xFFFFFFFFFFFFFFFF
		No check on buffer size, it should have enough
	*/
	unsigned int idx = 0;

	s->t.id = TINTEGER;
	incr(s);

	while (LETTER(*s->src) || 
		   NUMBER(*s->src)) 
	{
		if (idx == 18)
			opp_error(s, "Hex numbers cannot be above 18 characters (max 0xFFFFFFFFFFFFFFFF)");
		s->t.buffer[idx] = *s->src;
		idx++;
		incr(s);
	}
	s->t.buffer[idx] = '\0';
	s->t.value.num = (int64_t)hex2i64(s->t.buffer);
}

static void opp_lex_numeral(struct Opp_Scan* s)
{
	/* Max numeral length is 20 bytes:
		18446744073709551615
		No check on buffer size since it should not exceed this
	*/
	unsigned int idx = 0;
	bool has_dec = false;

	incr(s);
	if (*s->src == 'x' || 
		*s->src == 'X') 
	{
		opp_lex_hex(s);
		return;
	}
	decr(s);

	s->t.id = TINTEGER;
	
	while (NUMBER(*s->src) || 
		   *s->src == '.') 
	{
		if (idx == 20)
			opp_error(s, "Number size exceeds limit of 20 bytes");

		if (*s->src == '.') {
			if (has_dec)
				opp_error(s, "Unexpected '.' found");
			s->t.id = TFLOATING;
			has_dec = true;
		}

		s->t.buffer[idx] = *s->src;
		idx++;
		incr(s);
	}
	s->t.buffer[idx] = '\0';

	switch (s->t.id)
	{
		case TINTEGER: 
			s->t.value.num = atol(s->t.buffer);
			break;

		case TFLOATING:
			s->t.value.real = atof(s->t.buffer);
			break;

		default: break;
	}
}

static char opp_lex_escape(struct Opp_Scan* s)
{
	switch (*s->src) {
		case 'n':  return '\n';
		case 't':  return '\t';
		case '\\': return '\\';
		case '\'': return '\'';
		case '"':  return '\"';
		case 'v':  return '\v';
		case 'b':  return '\b';
		case 'r':  return '\r';
		case 'f':  return '\f';
		case 'a':  return '\a';
		case '?':  return '\?';
		case '0':  return '\0';

		default:
			opp_error(s, "Invalid escape character '%c'", *s->src);
	}
}

static void opp_lex_str(struct Opp_Scan* s)
{
	unsigned int idx = 0;
	
	incr(s);

	while (*s->src != '"') {
		if (!(*s->src) || *s->src == '\n')
			opp_error(s, "Expected terminating '\"' in string");

		else if (*s->src == '\\') {
			incr(s);
			write_into_buf(s, idx, opp_lex_escape(s));
		}
		else {
			write_into_buf(s, idx, *s->src);
		}
		idx++;
		incr(s);
	}
	incr(s);
	write_into_buf(s, idx, '\0');
	s->t.id = TSTR;
}

static void opp_lex_tik(struct Opp_Scan* s)
{
	incr(s);

	s->t.value.num = 0;
	s->t.id = TINTEGER;

	if (*s->src == '\\') {
		incr(s);
		s->t.value.num = (int64_t)opp_lex_escape(s);
	} 
	else {
		s->t.value.num = (int64_t)*s->src;
	}
	incr(s);

	if (*s->src != '\'')
		opp_error(s, "Terminating ' not found");

	incr(s);
}

#define CHECK(c) (*s->src != 0) && (*s->src == c)
static enum Opp_Token opp_lex_char(struct Opp_Scan* s)
{
	switch (*s->src)
	{
		case ';': {
			return TSEMICOLON;
		}

		case '(': {
			return TOPENP;
		}

		case ')': {
			return TCLOSEP;
		}

		case '{': {
			return TOPENC;
		}

		case '}': {
			return TCLOSEC;
		}

		case '[': {
			return TOPENB;
		}

		case ']': {
			return TCLOSEB;
		}

		case ',': {
			return TCOMMA;
		}
		
		case '|': {
			return TOR;
		}

		case '&': {
			return TADDR;
		}

		case '~': {
			return TBITNOT;
		}

		case '^': {
			return TBITXOR;
		}

		case '.': {
			incr(s);
			if (CHECK('.')) {
				incr(s);
				if (CHECK('.'))	
					return TVA_ARGS;
				break;
			}
			decr(s);
			return TDOT;
		}

		case '>': {
			incr(s);
			if (CHECK('='))
				return TGE;
			else if (CHECK('>')) 
				return TSHR;
			decr(s);
			return TGT;
		}

		case '<': {
			incr(s);
			if (CHECK('=')) 
				return TLE;
			else if (CHECK('<')) 
				return TSHL;
			decr(s);
			return TLT;
		}

		case '=': {
			incr(s);
			if (CHECK('=')) 
				return TEQEQ;
			decr(s);
			return TEQ;
		}

		case '!': {
			incr(s);
			if (CHECK('=')) 
				return TNOTEQ;
			decr(s);
			return TNOT;
		}

		case '+': {
			incr(s);
			if (CHECK('=')) 
				return TADDEQ;
			else if (CHECK('+')) 
				return TINCR;
			decr(s);
			return TADD;
		}

		// case '-': {
		// 	INCR;
		// 	if (CHECK('=')) return TMINEQ;
		// 	else if (CHECK('-')) return TDECR;
		// 	else if (CHECK('>')) return TARROW;
		// 	DECR;
		// 	return TMIN;
		// }

		// case '*': {
		// 	INCR;
		// 	if (CHECK('=')) return TMULEQ;
		// 	DECR;
		// 	return TMUL;
		// }

		// case '/': {
		// 	INCR;
		// 	if (CHECK('=')) return TDIVEQ;
		// 	DECR;
		// 	return TDIV;
		// }

		// case '%': {
		// 	INCR;
		// 	if (CHECK('=')) return TMODEQ;
		// 	DECR;
		// 	return TMOD;
		// }
		default: break;
	}
	return INVALID;
}

// enum Opp_Token opp_peek_tok(struct Opp_Scan* s, int times)
// {
// 	struct Opp_Scan cpy = *s;
// 	strcpy(s->peek, s->t.buffer.buf);

// 	for (int i = 0; i < times; i++)
// 		opp_next(s);

// 	enum Opp_Token t = s->t.id;

// 	// Swap
// 	for (unsigned int i = 0; i < s->t.buffer.len; i++) {
// 		char t = s->peek[i];
// 		s->peek[i] = s->t.buffer.buf[i];
// 		s->t.buffer.buf[i] = t;
// 	}
// 	*s = cpy;

// 	return t;
// }

void opp_next(struct Opp_Scan* s)
{
	memset(s->t.buffer, 0, s->t.size);

	while (*s->src)
	{
		if (*s->src == '\n') {
			s->line++;
			s->colum = 0;
		}
		else if (WHITESPACE(*s->src)) {}
		// else if (*s->src == '/' && s->src[1] == '/') {
		// 	while (*s->src && *s->src != '\n')
		// 		INCR; COMMENTS!!!!!!

		// 	s->line++;
		// 	s->colum = 0;
		// }
		// else if (*s->src == '/' && s->src[1] == '*') {
		// 	INCR;
		// 	INCR;
		// 	while (*s->src) {
		// 		if (*s->src == '*' && s->src[1] == '/')
		// 			break;
		// 		else if (*s->src == '\n') {
		// 			s->line++;
		// 			s->colum = 0;
		// 		}
		// 		INCR;
		// 	}
		// 	INCR;
		// }
		else if (*s->src == '\"') {
			opp_lex_str(s);
			return;
		}
		else if (*s->src == '\'') {
			opp_lex_tik(s);
			return;
		}
		else if (LETTER(*s->src)) {
			opp_lex_identifier(s); 
			return;
		}
		else if (NUMBER(*s->src)) {
			opp_lex_numeral(s);
			return;
		}
		else {
			s->t.id = opp_lex_char(s);
			if (s->t.id == INVALID) 
				opp_error(s, "Invalid token found");
			incr(s);
			return;
		}
		incr(s);
	}
	s->t.id = FEND;
}

void dump_tokens(struct Opp_Scan* s)
{
	do {
		opp_next(s);
		printf("%s:%d:%d\t", s->io.fname, s->line, s->colum);
		if (s->t.id == TFLOATING)
			printf("FLOAT\t %lf\n", s->t.value.real);
		else if (s->t.id == TINTEGER)
			printf("INT\t %lld\n", s->t.value.num);
		else if (s->t.id == TIDENT)
			printf("IDENT\t %s\n", s->t.buffer);
		else if (s->t.id == TSTR)
			printf("STRING\t \"%s\"\n", s->t.buffer);
		else if (s->t.id > TK_CHARS && s->t.id < TK_KEYWORDS)
			printf("CHAR\t '%s'\n", opp_keys[s->t.id].key);
		// else if (s->t.id >= TAUTO && s->t.id <= TINLINE)
		// 	printf("KEYWORD\t '%s'\n", opp_keys[s->t.id].key);
		else if (s->t.id == FEND)
			printf("FILE END\n");

	} while (s->t.id != FEND);
}