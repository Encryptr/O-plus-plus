/** @file platform.h
 * 
 * @brief Opp Platform Settings
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

#ifndef OPP_PLATFORM
#define OPP_PLATFORM

/*** Memory Allocation ***/

/* Size of block in bytes allocated by memory module. */
#define BLOCK_SIZE 64000

/* Alligment used by allocation function */
#define ALLIGMENT 8

/*** Lexer ***/

/* Size of buffer allocated for storing strings found while creating 
   tokens. If a string that is longer then this amount is detecting
   a reallocation is preformed adding this value to itself as the new 
   buffer size.
*/
#define SCAN_BUFFER_INITAL 32

/*** Parser ***/

/* Size of default amount of space allocated for parser statments
   AST nodes. This value also represents the incrementation amount 
   when needed to preform a reallocation.
*/ 
#define STMT_SIZE 64

/* Size of type hashmap */
#define TYPE_MAP_SIZE 64

/*** Analizer ***/
#define GLOABL_MAP_SIZE 64
#define LOCAL_MAP_SIZE 32

#endif