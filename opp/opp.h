/** @file opp.h
 * 
 * @brief Main opp header
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

#ifndef OPP_HEADER
#define OPP_HEADER

#include <setjmp.h>

struct Opp_Options {
	bool dump_toks,
		run_output,
		link_c,
		nostd,
		warning,
		debug,
		wall,
		test,
		live;
};

enum Opp_Global_State {
	OPP_OK,
	OPP_ERROR
};

struct Opp_State {
	struct Opp_Options opts;
	jmp_buf error_buf;
};

extern struct Opp_State global_state;

#endif /* OPP_HEADER */