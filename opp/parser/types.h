/** @file types.h
 * 
 * @brief Opp C Type Module
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
 */

#ifndef OPP_TYPES
#define OPP_TYPES

#include <stdio.h>

struct Opp_Type {
	enum Opp_Token type,
				   storage_class;
	unsigned char sign;
	unsigned char const_attr : 1;
	unsigned char volatile_attr : 1;
	unsigned char inline_attr : 1;
	unsigned char restrict_attr : 1;
	// char ptr_attr;
	// char ptr_volatile_attr;
	// char ptr_restrict_attr;
	struct Opp_Type_Entry* entry;
};

struct Opp_Type_Entry {
	int a;
};

struct Opp_Type_Tree {
	struct Opp_Type_Entry** tree;
	size_t size;
};

#endif /* OPP_TYPES */