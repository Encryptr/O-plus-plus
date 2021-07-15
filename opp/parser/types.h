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
#include <stdbool.h>

#include "../lexer/lexer.h"

enum Opp_Type_T {
	TYPE_NONE,
	TYPE_VOID,
	TYPE_CHAR,
	TYPE_SHORT,
	TYPE_INT,
	TYPE_LONG,
	TYPE_FLOAT,
	TYPE_DOUBLE,
	TYPE_LDOUBLE,
	TYPE_PTR,
	TYPE_FUNC,
	TYPE_ARRAY,
	TYPE_STRUCT,
	TYPE_UNION,
	TYPE_ENUM,
};

struct Opp_Func_Mem {
	const char* name;
	struct Opp_Type* type;
};

struct Opp_Type_Func {
	bool is_vaarg, incomplete;
	struct Opp_Func_Mem* param;
	unsigned int len;
};

struct Opp_Type_Obj {
	bool is_complete;
	struct Opp_Func_Mem* mems;
	unsigned int len;
};

struct Opp_Type {
	enum Opp_Type_T type;
	enum Opp_Token storage_class;
	bool unsign : 1,
		 const_attr : 1,
		 volatile_attr : 1,
		 inline_attr : 1,
		 restrict_attr : 1,
		 const_ptr : 1,
		 restrict_ptr : 1,
		 volatile_ptr : 1;

	union {
		struct Opp_Type_Func fn;
		struct Opp_Bucket* obj;
	} val;

	struct Opp_Type* next;
};

void opp_debug_type(struct Opp_Type* type);
struct Opp_Type* opp_create_type(enum Opp_Type_T type, struct Opp_Type* prev);
struct Opp_Type* opp_type_fix(struct Opp_Type* a, struct Opp_Type* b);

#endif /* OPP_TYPES */