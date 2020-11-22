/** @file io.h
 * 
 * @brief File IO header
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

#ifndef OPP_IO
#define OPP_IO

#include <stdio.h>

struct Opp_IO {
	const char* fname;
	FILE* file;
	long fsize;
};

typedef struct Opp_IO OppIO;

#endif /* OPP_IO */
