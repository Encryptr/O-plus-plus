/** @file env.h
 * 
 * @brief Opp compiler enviroment
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

#ifndef OPP_ENV
#define OPP_ENV

#include <stdint.h>
#include "../lexer/lexer.h"
#include "../ast/ast.h"

#define STACK_SIZE 8
#define HASH_SIZE 320

enum Opp_Bucket_Type {
	TYPE_GLOBAL, TYPE_LOCAL,
	TYPE_FUNC, TYPE_LABEL,
	TYPE_EXTERN
};

struct Opp_Function {
	struct Opp_Stmt_Func* args;
};

struct Opp_Struct {

};

struct Opp_Bucket {
	char* key;
	enum Opp_Bucket_Type type;
	unsigned int sym_idx;
	struct Opp_Type_Decl sym_type;
	union {
		struct Opp_Stmt_Func* args;
	};
	struct Opp_Bucket* next;
};

struct Opp_Namespace {
	size_t size;
	struct Opp_Bucket** items;
	struct Opp_Namespace* parent;
	void* (*xmalloc)(size_t);
};

unsigned int hash_str(char *string, unsigned int size);
struct Opp_Namespace* init_namespace(struct Opp_Namespace* parent, void* (*xmalloc)(size_t));
struct Opp_Bucket* env_add_item(struct Opp_Namespace* ns, char* name);
struct Opp_Bucket* env_get_item(struct Opp_Namespace* ns, char* name);
void env_free(struct Opp_Namespace* ns);

#endif /* OPP_ENV */