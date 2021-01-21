/* @file ir-linux.h
 * 
 * @brief Opp IR linux definitions
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

#ifndef IR_LINUX
#define IR_LINUX

#define DEFAULT_SECT 7

enum Linux_Sections {
	SECT_TEXT, SECT_DATA,
	SECT_SYMTAB, SECT_RELA_TEXT, 
	SECT_SHSTRTAB, SECT_STRTAB
};

#define DEFAULT_SYMS 16
struct Elf_Pair {
	struct Elf64_Symbol syms;
	const char* name;
};

struct Elf_Syms {
	size_t allocated;
	struct Elf_Pair* pairs;
	unsigned int sym_idx;
};

void strtable_write(unsigned int len, char* bytes);
void check_syms();


struct Elf_Pair* get_sym(unsigned int idx);
unsigned int get_str_idx();

void init_elf_syms();
void init_strtab();

void init_elf_header();
void init_text_sect(struct OppIr* ir);
void init_data_sect(struct OppIr* ir);
void init_symtab_sect();
void init_shstrtab_sect();
void init_strtab_sect();
void init_rela_text_sect();

void elf_offsets(struct OppIr* ir);

// Write elf file
void write_elf64(struct OppIr* ir, OppIO* io);

#endif /* IR_LINUX */