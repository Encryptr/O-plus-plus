// /** @file preprocessor.h
//  * 
//  * @brief Opp Preprocessor
//  *      
//  * Copyright (c) 2020 Maks S
//  *
//  * Licensed under the Apache License, Version 2.0 (the "License");
//  * you may not use this file except in compliance with the License.
//  * You may obtain a copy of the License at
//  *
//  * http://www.apache.org/licenses/LICENSE-2.0
//  *
//  * Unless required by applicable law or agreed to in writing, software
//  * distributed under the License is distributed on an "AS IS" BASIS,
//  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  * See the License for the specific language governing permissions and
//  * limitations under the License.
// **/

// #ifndef OPP_PREPROCESSOR
// #define OPP_PREPROCESSOR

// #include <stdio.h>
// #include "../lexer/lexer.h"
// #include "../util/util.h"

// #define MACRO_LEN 12
// struct Opp_Macro {
//     const char* name;
//     enum {
//         OBJECT,
//         FUNCTION
//     } type;

//     unsigned int params;
//     char** replace;
// 	char* body;

//     struct Opp_Macro* next;
// };

// struct Opp_Source {
//     uint32_t line, colum;
//     char* content, *newbuf, *ptr;
//     unsigned int allocated, idx;
//     struct Opp_Tok tok;
// };

// struct Opp_State {
//     const char* date;
//     char time[9];
// };

// struct Opp_Scanner {
//     struct Opp_IO io;
//     struct Opp_Macro** table;
// 	struct Opp_Source src;
//     struct Opp_State state;
// };

// // Preprocessor
// struct Opp_Scanner* opp_init_scanner(const char* fname);
// void opp_start_preprocessor(struct Opp_Scanner* s);

// #endif /* OPP_PREPROCESSOR */