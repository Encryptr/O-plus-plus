/* @file ir-linux.c
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

#include "ir.h"

#ifdef LINUX64

struct Elf_Info {
	int sect_amount;

	struct Elf64_Header elf64_hdr;
	struct Elf64_SHDR* sect_list;

	struct {
		size_t allocated;
		unsigned char* str;
		unsigned int str_idx;
	} strtable;
	struct Elf_Syms symbols;
};

static struct Elf_Info elf_info;

static unsigned char shstrtab[] = {
	// .text
	0x0, 0x2E, 0x74, 0x65, 0x78, 0x74,
	// .data
	0x0, 0x2E, 0x64, 0x61, 0x74, 0x61, 
	// .shstrtab
	0x0, 0x2E, 0x73, 0x68, 0x73, 0x74, 0x72, 0x74, 0x61, 0x62, 
	// .symtab
	0x0, 0x2E, 0x73, 0x79, 0x6D, 0x74, 0x61, 0x62, 
	// .strtab
	0x0, 0x2E, 0x73, 0x74, 0x72, 0x74, 0x61, 0x62, 
	// .rela.text
	0x0, 0x2E, 0x72, 0x65, 0x6C, 0x61, 0x2E, 0x74, 0x65, 0x78, 0x74, 
	0x0
};

void check_syms()
{
}

struct Elf_Pair* get_sym(unsigned int idx)
{
	if (idx >= elf_info.symbols.allocated) {
		printf("REALLOC SYM\n");
	}

	elf_info.symbols.sym_idx++;
	return &elf_info.symbols.pairs[idx];
}

void strtable_write(unsigned int len, char* bytes)
{
	if ((elf_info.strtable.str_idx + (len+2)) >= elf_info.strtable.allocated) {
		printf("REALLOC STRTABLE\n");
	}
	elf_info.strtable.str[elf_info.strtable.str_idx++] = 0x0;
	for (unsigned int i = 0; i < len; i++)
		elf_info.strtable.str[elf_info.strtable.str_idx++] = bytes[i];
}

unsigned int get_str_idx()
{
	return elf_info.strtable.str_idx + 1;
}

void init_strtab()
{
	elf_info.strtable.str = (unsigned char*)malloc(INIT_BYTECODE_SIZE);

	if (elf_info.strtable.str == NULL)
		INTERNAL_ERROR("Malloc fail");

	elf_info.strtable.allocated = INIT_BYTECODE_SIZE;
	elf_info.strtable.str_idx = 0;
	memset(elf_info.strtable.str, 0, INIT_BYTECODE_SIZE);
}

void init_elf_syms(OppIO* io)
{
	elf_info.symbols.pairs = (struct Elf_Pair*)
		malloc(sizeof(struct Elf_Pair)*DEFAULT_SYMS);

	if (elf_info.symbols.pairs == NULL)
		INTERNAL_ERROR("Malloc fail");

	elf_info.symbols.allocated = DEFAULT_SYMS;
	elf_info.symbols.sym_idx = 0;
	memset(elf_info.symbols.pairs, 0, sizeof(struct Elf_Pair)*DEFAULT_SYMS);

	struct Elf64_Symbol* sym = &elf_info.symbols.pairs[1].syms;
	elf_info.symbols.pairs[1].name = NULL;
	strtable_write(strlen("test.opp"), "test.opp");

	// ABS fname
	sym->st_name = 1;
	sym->st_value = 0;
	sym->st_size = 0;
	sym->st_info = 4;
	sym->st_other = 0;
	sym->st_shndx = 65521;
	// sym = &elf_info.symbols.pairs[2].syms;
	// // Text section
	// sym->st_name = 0;
	// sym->st_value = 0;
	// sym->st_size = 0;
	// sym->st_info = 3;
	// sym->st_other = 0;
	// sym->st_shndx = 1;
	// // Data Section
	// sym = &elf_info.symbols.pairs[3].syms;
	// sym->st_name = 0;
	// sym->st_value = 0;
	// sym->st_size = 0;
	// sym->st_info = 3;
	// sym->st_other = 0;
	// sym->st_shndx = 2;
	elf_info.symbols.sym_idx += 2;
}

void init_elf_header()
{
	elf_info.sect_list = (struct Elf64_SHDR*)
		malloc(sizeof(struct Elf64_SHDR)*DEFAULT_SECT);
 	
	if (elf_info.sect_list == NULL)
		INTERNAL_ERROR("Malloc fail");

	elf_info.sect_amount = DEFAULT_SECT;

	elf_info.elf64_hdr.e_ident[0] = EI_MAG0;
	elf_info.elf64_hdr.e_ident[1] = EI_MAG1;
	elf_info.elf64_hdr.e_ident[2] = EI_MAG2;
	elf_info.elf64_hdr.e_ident[3] = EI_MAG3;
	elf_info.elf64_hdr.e_ident[4] = ELFCLASS64;
	elf_info.elf64_hdr.e_ident[5] = ELFDATA2LSB;
	elf_info.elf64_hdr.e_ident[6] = EV_CURRENT;
	elf_info.elf64_hdr.e_ident[7] = ELFOSABI_NONE;
	elf_info.elf64_hdr.e_ident[8] = 0x0;

	elf_info.elf64_hdr.e_type = ET_REL;
	elf_info.elf64_hdr.e_machine = EM_X86_64;
	elf_info.elf64_hdr.e_version = EV_CURRENT;
	elf_info.elf64_hdr.e_entry = DEFAULT_ENTRY;
	elf_info.elf64_hdr.e_phoff = 0x0;
	elf_info.elf64_hdr.e_shoff = 64;
	elf_info.elf64_hdr.e_flags = 0x0;
	elf_info.elf64_hdr.e_ehsize = sizeof(elf_info.elf64_hdr);
	elf_info.elf64_hdr.e_phentsize = 0x0;
	elf_info.elf64_hdr.e_phnum = 0x0;
	elf_info.elf64_hdr.e_shentsize = 64;
	elf_info.elf64_hdr.e_shnum = DEFAULT_SECT;
	elf_info.elf64_hdr.e_shstrndx = 5;
}

void init_text_sect(struct OppIr* ir)
{
	struct Elf64_SHDR* sect = &elf_info.sect_list[SECT_TEXT];
	sect->sh_name = 1;
	sect->sh_type = SHT_PROGBITS;
	sect->sh_flags = 0x6;
	sect->sh_addr = 0x0;
	sect->sh_offset = 0x0;
	sect->sh_size = ir->code.idx;
	sect->sh_link = 0;
	sect->sh_info = 0;
	sect->sh_addralign = 0x8;
	sect->sh_entsize = 0;

	while (ir->code.idx % 8 != 0) {
		oppir_check_realloc(ir, 1);
		IR_EMIT(0x0);
	}
}

void init_data_sect(struct OppIr* ir)
{
	struct Elf64_SHDR* sect = &elf_info.sect_list[SECT_DATA];
	sect->sh_name = 7;
	sect->sh_type = SHT_PROGBITS;
	sect->sh_flags = 3;
	sect->sh_addr = 0x0;
	sect->sh_offset = 0x0;
	sect->sh_size = ir->data_seg.idx;
	sect->sh_link = 0;
	sect->sh_info = 0;
	sect->sh_addralign = 8;
	sect->sh_entsize = 0;

	while (ir->data_seg.idx % 8 != 0) {
		// oppir_check_realloc(ir, 1);
		printf("ADD DATA\n");
		break;
	}
}

void init_shstrtab_sect()
{
	struct Elf64_SHDR* sect = &elf_info.sect_list[SECT_SHSTRTAB];
	sect->sh_name = 13;
	sect->sh_type = 3;
	sect->sh_flags = 0;
	sect->sh_addr = 0x0;
	sect->sh_offset = 0x0;
	sect->sh_size = 50;
	sect->sh_link = 0;
	sect->sh_info = 0;
	sect->sh_addralign = 1;
	sect->sh_entsize = 0;
}

void init_symtab_sect()
{
	struct Elf64_SHDR* sect = &elf_info.sect_list[SECT_SYMTAB];
	sect->sh_name = 23;
	sect->sh_type = 2;
	sect->sh_flags = 0;
	sect->sh_addr = 0x0;
	sect->sh_offset = 0x0;
	sect->sh_size = sizeof(struct Elf64_Symbol)*elf_info.symbols.sym_idx;
	sect->sh_link = 6;
	sect->sh_info = 2; 
	sect->sh_addralign = 4;
	sect->sh_entsize = sizeof(struct Elf64_Symbol);
}

void init_strtab_sect()
{
	struct Elf64_SHDR* sect = &elf_info.sect_list[SECT_STRTAB];
	sect->sh_name = 31;
	sect->sh_type = 3;
	sect->sh_flags = 0;
	sect->sh_addr = 0x0;
	sect->sh_offset = 0x0;
	sect->sh_size = elf_info.strtable.str_idx;
	sect->sh_link = 0; 
	sect->sh_info = 0;
	sect->sh_addralign = 1;
	sect->sh_entsize = 0;
}

void init_rela_text_sect()
{
	struct Elf64_SHDR* sect = &elf_info.sect_list[SECT_RELA_TEXT];
	sect->sh_name = 39;
	sect->sh_type = 4;
	sect->sh_flags = 0;
	sect->sh_addr = 0x0;
	sect->sh_offset = 0x0;
	sect->sh_size = 0; // Reloc size * amount
	sect->sh_link = 3; 
	sect->sh_info = 1;
	sect->sh_addralign = 4;
	sect->sh_entsize = 24;
}

void elf_offsets(struct OppIr* ir)
{
	unsigned int offset = 
		sizeof(struct Elf64_Header) + 
		(sizeof(struct Elf64_SHDR) * elf_info.sect_amount);

	// Text / Data
	elf_info.sect_list[SECT_TEXT].sh_offset = offset;
	offset += elf_info.sect_list[SECT_TEXT].sh_size + 
		(ir->code.idx - elf_info.sect_list[SECT_TEXT].sh_size);

	elf_info.sect_list[SECT_DATA].sh_offset = offset;
	// do data

	for (int i = 2; i < elf_info.sect_amount-1; i++) {
		elf_info.sect_list[i].sh_offset = offset;
		offset += elf_info.sect_list[i].sh_size;
	}
}

void write_elf64(struct OppIr* ir, OppIO* io)
{
	// Header
	fwrite(&elf_info.elf64_hdr, sizeof(elf_info.elf64_hdr), 1, io->file);

	// Sections
	for (unsigned int i = 0; i < sizeof(struct Elf64_SHDR); i++) 
		fwrite(((char[1]){0}), 1, 1, io->file);
	fwrite(elf_info.sect_list, sizeof(struct Elf64_SHDR), elf_info.sect_amount-1, io->file);
	
	// Text
	fwrite(ir->code.bytes, 1, ir->code.idx, io->file);

	// Data
	fwrite(ir->data_seg.data, 1, ir->data_seg.idx, io->file);

	// Sym / Rela
	for (unsigned int i = 0; i < elf_info.symbols.sym_idx; i++)
		fwrite(&elf_info.symbols.pairs[i].syms, sizeof(struct Elf64_Symbol), 1, io->file);

	// Rela Text

	// Shstrtab
	fwrite(shstrtab, 1, elf_info.sect_list[SECT_SHSTRTAB].sh_size, io->file);

	// Strtab
	fwrite(elf_info.strtable.str, 1, elf_info.strtable.str_idx, io->file);
	char z = 0;
	while (elf_info.strtable.str_idx % 16 != 0) {
		fwrite(&z, 1, 1, io->file);
		elf_info.strtable.str_idx++;
	}
	fwrite(&z, 1, 1, io->file);
}

#endif