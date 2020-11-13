/** @file linux-header.h
 * 
 * @brief Linux OS Elf file format description
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

#ifndef ELF_HEADER
#define ELF_HEADER

#define EI_NIDENT 16

struct Elf64_Header { 
	unsigned char e_ident[EI_NIDENT];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint64_t e_entry;
	uint64_t e_phoff;
	uint64_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
};

#define MAG0 0x7F
#define MAG1 'E'
#define MAG2 'L'
#define MAG3 'F'
#define ELFCLASS64 0x02
#define ELFDATA2LSB 0x1
#define EV_CURRENT 0x1

#define ET_REL 0x1
#define EM_X86_64 0x3e
#define DEFAULT_ENTRY 0x0

struct Elf64_PHDR {
	uint32_t p_type;
	uint32_t p_flags;
	uint64_t p_offset;
	uint64_t p_vaddr;
	uint64_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
};

struct Elf64_SHDR {
	uint32_t sh_name;
	uint32_t sh_type;
	uint64_t sh_flags;
	uint64_t sh_addr;
	uint64_t sh_offset;
	uint64_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint64_t sh_addralign;
	uint64_t sh_entsize;
};

struct Elf64_Symbol {
   uint32_t st_name;
   unsigned char st_info;
   unsigned char st_other;
   uint16_t st_shndx;
   uint64_t st_value;
   uint64_t st_size;
};

#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHT_STRTAB 0x3
#define SHT_PROGBITS 0x1
#define SHT_SYMTAB 0x2

#endif /* ELF_HEADER */