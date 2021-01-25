/** @file analysis.h
 * 
 * @brief Preform analysis stage
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

#ifndef OPP_ANALYSIS
#define OPP_ANALYSIS

#include "../env/env.h"
#include "../parser/parser.h"
#include "../memory/memory.h"
#include "../ast/ast.h"

struct Opp_Debug_Func {
	char* scope;
	bool ret;
	struct Opp_Type_Decl ret_type;
	const char* res_name;
};

typedef struct Opp_Type_Decl Opp_Obj;

struct Opp_Analize {
	struct Opp_Options* opts;
	struct Opp_Parser* parser;
	struct Opp_Namespace* ns;
	struct Opp_Namespace* curr_ns;
	struct Opp_Debug_Func debug;
};

struct Opp_Analize* opp_init_analize(struct Opp_Parser* parser, struct Opp_Options* opts);
void analize_tree(struct Opp_Analize* ctx);


#endif /* OPP_ANALYSIS */