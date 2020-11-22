/** @file lexer.c
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

#include "lexer.h"

static inline bool ignore(char i)
{
	return (i == ' ' || i == '\t' || i=='\r');
}

static inline bool isletter(char i)
{
	return ((i >= 'a' && i <= 'z') || 
		(i >= 'A' && i <= 'Z') || (i == '_'));
}

static inline bool isnum(char i)
{
	return ((i >= '0' && i <= '9'));
}

uint64_t hex2i64(char *str)
{
	uint64_t res = 0;
	char c;

	while ((c = *str++)) {
		char v = (c & 0xF) + (c >> 6) | ((c >> 3) & 0x8);
		res = (res << 4) | (uint64_t) v;
	}

	return res;
}

void opp_error(struct Opp_Scan* s, const char* str, ...)
{
	va_list ap;
	va_start(ap, str);
	
	printf("%s:%u:%u ", s->io.fname, s->line, s->colum);
	#ifdef UNX
		printf(CL_RED);
	#endif

	printf("error: \n\t");
	printf(CL_RESET); 
	vfprintf(stdout, str, ap);
	fprintf(stdout, "\n");
	va_end(ap);
	exit(1);
}

void internal_error(const char* str, const char* func)
{
	#ifdef UNX 
	printf(CL_RED); 
	#endif 
	printf("** Error: %s [%s] **\n", str, func); 
	#ifdef UNX 
	printf(CL_RESET); 
	#endif 
	exit(1); 
}

struct Opp_Keywords {
	const char* key;
	enum Opp_Token id;
};

#define OPP_KEYOWRDS 15
static const struct Opp_Keywords opp_keywords[] = {
	{"asm",  TASM},
	{"auto", TAUTO},
	{"sizeof", TSIZEOF},
	{"struct", TSTRUCT},
	{"while", TWHILE},
	{"if", TIF},
	{"else", TELSE},
	{"switch", TSWITCH},
	{"return", TRET},
	{"goto", TGOTO},
	{"extern", TEXTERN},
	{"import", TIMPORT},
	{"for", TFOR},
	{"case", TCASE},
	{"break", TBREAK}
};

bool is_keyword(struct Opp_Scan* s)
{
	for (int i = 0; i < OPP_KEYOWRDS; i++) {
		if (!strcmp(opp_keywords[i].key, s->t.buffer.buf)) {
			s->t.id = opp_keywords[i].id;
			return 1;
		}
	}
	return 0;
}

void realloc_buf(struct Opp_Scan* s)
{
	s->t.buffer.len += 32;
	char* temp = realloc(s->t.buffer.buf, s->t.buffer.len);
	if (temp == NULL)
		INTERNAL_ERROR("malloc fail");
	s->t.buffer.buf = temp;
}

void init_opp_file(struct Opp_Scan* s, const char* fname)
{
	s->io.fname = fname;
	s->io.file = fopen(fname, "r");

	if (!s->io.file) {
		printf("Error opening file '%s'\n", fname);
		exit(1);
	}
}

void init_opp_lex(struct Opp_Scan* s, char* content)
{
	s->line = 1;
	s->colum = 0;
	s->src = content;
	s->t.buffer.buf = (char*)malloc(SCAN_BUF);
	if (s->t.buffer.buf == NULL)
		INTERNAL_ERROR("malloc fail");

	s->t.buffer.len = SCAN_BUF;
}

void opp_deinit(struct Opp_Scan* s)
{
	free(s->t.buffer.buf);
}

void opp_lex_identifier(struct Opp_Scan* s)
{
	unsigned int idx = 0;
	while (isletter(*s->src) || isnum(*s->src)) {

		if (idx > s->t.buffer.len)
			realloc_buf(s);

		s->t.buffer.buf[idx] = *s->src;
		idx++;
		INCR;
	}
	s->t.buffer.buf[idx] = '\0';
	if (is_keyword(s))
		return;
	s->t.id = TIDENT;
}

void opp_lex_hex(struct Opp_Scan* s)
{
	unsigned int idx = 0;
	s->t.id = TINTEGER;
	INCR;
	while (isletter(*s->src) || isnum(*s->src)) {
		s->t.buffer.buf[idx] = *s->src;
		idx++;
		INCR;
	}
	s->t.buffer.buf[idx] = '\0';
	s->t.num = hex2i64(s->t.buffer.buf);
}

void opp_lex_numeral(struct Opp_Scan* s)
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
	} 

	while (isnum(*s->src) || *s->src == '.') {
		if (*s->src == '.') s->t.id = TFLOAT;
		s->t.buffer.buf[idx] = *s->src;
		idx++;
		INCR;
	}
	s->t.buffer.buf[idx] = '\0';

	switch (s->t.id)
	{
		case TINTEGER: 
			s->t.num = atol(s->t.buffer.buf);
			break;

		case TFLOAT:
			s->t.real = atof(s->t.buffer.buf);
			break;
	}
}

void opp_peek_tok(struct Opp_Scan* s)
{
	char* temp = s->src;
	uint32_t tline = s->line, tcolum = s->colum;

	opp_next(s);

	s->src = temp;
	s->line = tline;
	s->colum = tcolum;
}

void opp_lex_str(struct Opp_Scan* s)
{
	INCR;
	unsigned int idx = 0;
	while (*s->src != '"') {
		if (idx >= s->t.buffer.len)
			realloc_buf(s);

		if (*s->src == EOF || *s->src == '\n')
			opp_error(s, "Expected terminating '\"' in string");
		else if (*s->src == '\\') {
			INCR;
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

				case '"':
					s->t.buffer.buf[idx] = '\"';
					break;

				default:
					opp_error(s, "Invalid escape character '%c'", *s->src);
			}
		}
		else
			s->t.buffer.buf[idx] = *s->src;
		idx++;
		s->src++;
		s->colum++;
	}
	INCR;
	s->t.buffer.buf[idx] = '\0';
	s->t.id = TSTR;
}

enum Opp_Token opp_lex_char(struct Opp_Scan* s)
{
	switch (*s->src)
	{
		case '(': return TOPENP;
		case ')': return TCLOSEP;
		case ',': return TCOMMA; 
		case '[': return TOPENB;
		case ']': return TCLOSEB;
		case ';': return TSEMICOLON; 
		case '{': return TOPENC;
		case '}': return TCLOSEC;
		case '.': return TDOT;
		case ':': return TCOLON;

		case '-':
			INCR;
			if (*s->src == '-') 
				return TDECR;
			DECR;
			return TMIN;

		case '+':
			INCR;
			if (*s->src == '+') 
				return TINCR;
			DECR;
			return TADD;
			
		case '>': 
			INCR; 
			if (*s->src == '=') 
				return TGE; 
			DECR; 
			return TGT;

		case '<': 
			INCR; 
			if (*s->src == '=') 
				return TLE; 
			DECR; 
			return TLT;
		
		case '&': 
			INCR; 
			if (*s->src == '&') 
				return TAND; 
			DECR; 
			return TADDR;

		case '|': 
			INCR; 
			if (*s->src == '|') 
				return TOR; 
			DECR; 
			break;

		case '=': 
			INCR; 
			if (*s->src == '=') 
				return TEQEQ; 
			DECR; 
			return TEQ;

		case '!': 
			INCR; 
			if (*s->src == '=') 
				return TNOTEQ; 
			DECR; 
			return TNOT;

		case '/': 
			INCR;
			 if (*s->src == '=') 
				return TDIVEQ; 
			DECR; 
			return TDIV;

		case '*': 
			INCR; 
			if (*s->src == '=') 
				return TMULEQ; 
			DECR; 
			return TMUL;

		case '%': 
			INCR; 
			if (*s->src == '=') 
				return TMODEQ; 
			DECR; 
			return TMOD;
		
	}
	return INVALID;
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
		else if (*s->src == '/') {
			INCR;
			if (*s->src == '/') {
				while (*s->src != '\n' && *s->src) {
					s->src++;
				}
			}
			else {
				s->t.id = TDIV;
				return;
			}
			s->colum = 0;
			s->line++;
		}
		else if (isletter(*s->src)) {
			opp_lex_identifier(s); 
			return;
		}
		else if (*s->src == '\"') {
			opp_lex_str(s);
			return;
		}
		else if (isnum(*s->src)) {
			opp_lex_numeral(s);
			return;
		}
		else {
			enum Opp_Token res = opp_lex_char(s);
			if (res == INVALID) 
				opp_error(s, "Invalid token found '%c'", *s->src);
			s->t.id = res;
			INCR;
			return;
		}

		s->colum++;
		s->src++;
	}
	s->t.id = FEND;
}

void dump_tokens(struct Opp_Scan* s)
{
	do {
		opp_next(s);
		printf("%s:%d:%d ", s->io.fname, s->line, s->colum);
		if (s->t.id == TFLOAT)
			printf("FLOAT %lf\n", s->t.real);
		else if (s->t.id == TINTEGER)
			printf("INT %ld\n", s->t.num);
		else if (s->t.id == TIDENT)
			printf("IDENT %s\n", s->t.buffer.buf);
		else if (s->t.id == TSTR)
			printf("STRING \"%s\"\n", s->t.buffer.buf);
		else if (s->t.id > 5 && s->t.id <= 38)
			printf("CHAR %d\n", s->t.id);

	} while (s->t.id != FEND);
}