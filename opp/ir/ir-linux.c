#include "ir.h"

#ifdef LINUX64
void init_elf_deps()
{
	strtable = (unsigned char*)malloc(1 * 64);
	

}

void init_elf_header(int sections)
{
	sect_list = (struct Elf64_SHDR*)
		malloc(sizeof(struct Elf64_SHDR)*sections);

	if (sect_list == NULL)
		INTERNAL_ERROR("Malloc fail");

	elf64_hdr.e_ident[0] = EI_MAG0;
	elf64_hdr.e_ident[1] = EI_MAG1;
	elf64_hdr.e_ident[2] = EI_MAG2;
	elf64_hdr.e_ident[3] = EI_MAG3;
	elf64_hdr.e_ident[4] = ELFCLASS64;
	elf64_hdr.e_ident[5] = ELFDATA2LSB;
	elf64_hdr.e_ident[6] = EV_CURRENT;
	elf64_hdr.e_ident[7] = ELFOSABI_NONE;
	elf64_hdr.e_ident[8] = 0x0;

	elf64_hdr.e_type = ET_REL;
	elf64_hdr.e_machine = EM_X86_64;
	elf64_hdr.e_version = EV_CURRENT;
	elf64_hdr.e_entry = DEFAULT_ENTRY;
	elf64_hdr.e_phoff = 0x0;
	elf64_hdr.e_shoff = 64;
	elf64_hdr.e_flags = 0x0;
	elf64_hdr.e_ehsize = sizeof(elf64_hdr);
	elf64_hdr.e_phentsize = 0x0;
	elf64_hdr.e_phnum = 0x0;
	elf64_hdr.e_shentsize = 64;
	elf64_hdr.e_shnum = DEFAULT_SECT;
	elf64_hdr.e_shstrndx = 3;
}

void init_text_sect(struct OppIr* ir)
{
	struct Elf64_SHDR* sect = &sect_list[SECT_TEXT];

	sect->sh_name = 1;
	sect->sh_type = SHT_PROGBITS;
	sect->sh_flags = 0x6;
	sect->sh_addr = 0x0;

	// Offsets
	sect->sh_offset = 0x0;

	sect->sh_size = ir->code.idx;
	
}

void init_symtab_sect()
{

}

void init_shstrtab_sect()
{

}

void init_strtab_sect()
{

}

void init_rela_text_sect()
{

}

#endif