/** @file ir-mac.h
 * 
 * @brief MacOS ir
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

#ifdef MAC64
static struct Mach_Header_64 macho_hdr;
static struct Section_64 sects[2];
static struct Macho_Reloc* reloc_data;
static struct Macho_Data data_dump;
static struct Macho_Symbol sym_data;
static unsigned char* string_table;

void init_sym_list()
{
	
}

void init_sect_list()
{

}

void init_reloc_list()
{

}

void init_data()
{
	string_table = (unsigned char*)
		malloc(DEFAULT_DUMP);

	if (string_table == NULL)
		INTERNAL_ERROR("Malloc fail");

	data_dump.data = (unsigned char*)
		malloc(DEFAULT_DUMP);

	if (data_dump.data == NULL)
		INTERNAL_ERROR("Malloc fail");

	data_dump.allocated = DEFAULT_DUMP;
	data_dump.idx = 0;	
}
#endif