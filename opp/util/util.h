/** @file util.h
 * 
 * @brief Opp Utilities
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
**/

#ifndef OPP_UTIL
#define OPP_UTIL

#include <stdio.h>
#include <stdlib.h>
#include "../lexer/lexer.h"

#define INTERNAL_ERROR(str) \
	internal_error(str)
#define MALLOC_FAIL() \
	INTERNAL_ERROR("Malloc fail")

// Error Util
void opp_error(struct Opp_Scan* s, const char* str, ...);
void internal_error(const char* str);

#endif /* OPP_UTIL */