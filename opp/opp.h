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

#ifndef OPP_H
#define OPP_H

#include "./compiler/compiler.h"
#include "./ir/ir.h"
#include "./header/os.h"

struct Opp_Scan;
struct Opp_Options;

void opp_init_file(const char* fname, struct Opp_Scan* s);
void opp_init_module(const char* fname, struct Opp_Options* opts);
// void opp_new_module(const char* fname, struct Opp_Context* opp);

#endif /* OPP_H */