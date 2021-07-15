/** @file compiler.c
 * 
 * @brief Opp Compiler
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
**/

#include "compiler.h"
#include "../memory/memory.h"
#include "../util/util.h"

struct Opp_Compiler* opp_new_compiler(struct Opp_Parser* parser)
{
	struct Opp_Compiler* opp = (struct Opp_Compiler*)
		opp_alloc(sizeof(struct Opp_Compiler));

	if (!opp)
		MALLOC_FAIL();

	opp->parser = parser;

	return opp;
}

void opp_start_compiler(struct Opp_Compiler* opp)
{
	// begin by analyzing top level decl and func 
	// decl is converted to no byte code since its assigment expr is const
	// func entry is created and bytecode is began created from it.
}