/** @file types.h
 * 
 * @brief Type tree module
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

#ifndef OPP_TYPES
#define OPP_TYPES

enum Opp_Std_Type {
	TYPE_VOID,
	TYPE_I8,
	TYPE_I16,
	TYPE_I32,
	TYPE_I64,
	TYPE_FLOAT,
	TYPE_DOUBLE,
	TYPE_BITFIELD,
	TYPE_ARRAY,
	TYPE_STRUCT,
};

struct Opp_Struct {
	enum Opp_Std_Type* elems;
	unsigned int count;
};

struct Opp_Type_Entry {
	char* id;
	enum Opp_Std_Type t_type;
	struct Opp_Struct s_type;
	struct Opp_Type_Entry* next;
};

struct Opp_Type_Tree {
	struct Opp_Type_Entry** types;
	size_t size;
};

#endif /* OPP_TYPES */