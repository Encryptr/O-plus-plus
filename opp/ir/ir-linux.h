/** @file ir-linux.h
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

#define DEFAULT_SECT 6

enum Linux_Sections {
	SECT_TEXT, SECT_DATA,
	SECT_SHSTRTAB, SECT_SYMTAB, 
	SECT_STRTAB, SECT_RELA_TEXT
};

struct Elf_Pair {
	struct Elf64_Symbol syms;
	char* name;
};

struct Elf_Syms {
	struct Elf_Pair* pairs;
	unsigned int sym_idx;
};

static struct Elf64_SHDR* sect_list;
static struct Elf_Syms symbols;

// Basic Elf segments
static unsigned char* strtable;
static struct Elf64_Header elf64_hdr;

static unsigned char setup_bytes[] = {
	0xe8, 0x0, 0x0, 0x0, 0x0,
	0x48, 0x89, 0xc7,
	0xb8, 0x3c, 0x0, 0x0, 0x0,
	0x0f, 0x05
};


void init_elf_deps();
void init_elf_header();
void init_text_sect(struct OppIr* ir);
void init_symtab_sect();
void init_shstrtab_sect();
void init_strtab_sect();
void init_rela_text_sect();

// void elf_offsets(struct OppIr* ir);

// // Write elf file
// void write_elf64_header(OppIO* io);




#endif /* IR_LINUX */