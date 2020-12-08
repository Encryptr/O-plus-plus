#include "ir.h"

#define PUB_FUNC 0x0F
#define DEFAULT_DUMP 64

enum Macho_Sect_Type {
	SECT_TEXT, SECT_DATA
};

enum Reloc_Loc {
	RELOC_DATA,
	RELOC_TEXT,
};


struct Macho_Symbol {

};

struct Ir_Reloc {
	enum Reloc_Loc loc; 
	char* sym_name;
	struct Relocation reloc;
};

struct Macho_Reloc {
	struct Ir_Reloc* list;
	size_t allocated;
	unsigned int reloc_idx;
};

struct Macho_Data {
	unsigned char* data;
	size_t allocated;
	unsigned int idx;
};

void init_sym_list();
void init_sect_list();
void init_reloc_list();
void init_data();

// // Setup macho headers
// void create_symbol_table();
// void init_macho_header();
// void init_segment_command(struct OppIr* ir);
// void init_text_section(struct OppIr* ir);
// void init_symtab();
// void init_string_table();
// void init_header_offsets(struct OppIr* ir);

// // Write macho headers
// bool oppir_output_file(const char* fname, OppIO* io);
// void write_macho_header(OppIO* io);
// void write_segment_header(OppIO* io);
// void write_sections(OppIO* io);
// void write_symtab(OppIO* io);
// void write_machine_code(OppIO* io, struct OppIr* ir);
// void write_symbols(OppIO* io);
// void write_reloc_section(OppIO* io);
// void write_stringtab(OppIO* io);
