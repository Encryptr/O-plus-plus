/** @file mac-header.h
 * 
 * @brief MacOS Macho file format description
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

#ifndef MACHO_HEADER
#define MACHO_HEADER

struct Mach_Header_64 {
	uint32_t magic;
	uint32_t cputype;
	uint32_t cpusubtype;
	uint32_t filetype;
	uint32_t ncmds;
	uint32_t sizeofcmds;
	uint32_t flags;
	uint32_t reserved;
};

#define MAGIC_64 0xfeedfacf
#define CPU_TYPE_X86 0x01000007
#define SUB_SYS 0x00000003
#define FILE_MH_OBJ 0x00000001

/* Load Commands */
#define	LC_SEGMENT_64 0x19
#define	LC_SYMTAB 0x2

struct Load_Cmd {
	uint32_t cmd;		
	uint32_t cmdsize;
};

struct Symtab_Cmd {
	uint32_t cmd;		
	uint32_t cmdsize;	
	uint32_t symoff;		
	uint32_t nsyms;		
	uint32_t stroff;		
	uint32_t strsize;	
};

struct Nlist_64 { 
	union { 
		uint32_t n_strx; 
	} n_un;
	uint8_t n_type; 
	uint8_t n_sect; 
	uint16_t n_desc; 
	uint64_t n_value; 
};

struct Segment_Cmd_64 { 
	uint32_t cmd;		
	uint32_t cmdsize;	
	char segname[16];
	uint64_t vmaddr;		
	uint64_t vmsize;		
	uint64_t fileoff;	
	uint64_t filesize;	
	uint32_t maxprot;	
	uint32_t initprot;	
	uint32_t nsects;		
	uint32_t flags;		
};

struct Section_64 { 
	char sectname[16];	
	char segname[16];	
	uint64_t addr;		
	uint64_t size;		
	uint32_t offset;		
	uint32_t align;		
	uint32_t reloff;		
	uint32_t nreloc;		
	uint32_t flags;		
	uint32_t reserved1;	
	uint32_t reserved2;	
	uint32_t reserved3;	
};

struct Relocation {
	int32_t r_address; 
	uint32_t r_symbolnum:24; 
	uint32_t r_pcrel:1;
	uint32_t r_length:2;
	uint32_t r_extern:1;
	uint32_t r_type:4;
};

#endif /* MACHO_HEADER */