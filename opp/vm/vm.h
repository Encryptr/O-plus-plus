/** @file vm.h
 * 
 * @brief Opp VM
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

#ifndef OPP_VM
#define OPP_VM

#include "../parser/types.h"

struct Opp_Value {
	enum Opp_Type_T type;
	union {
		int64_t i_num;
		uint64_t u_num;
		double f64;
		float f32;
		void* raw_ptr;
		struct Opp_Value* ptr;
	} v;
};

struct Opp_VM {
	int i;
};

struct Opp_VM* opp_new_vm();

#endif /* OPP_VM */