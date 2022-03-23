/** @file platform.h
 * 
 * @brief Opp Platform Settings
 *      
 * Copyright (c) 2022 Maks S
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

#if defined(_WIN64)
   #define SYSTEM_WIN_64
#elif defined(__APPLE__) || defined(__MACH__)
   #define SYSTEM_MAC_64
   #define SYSTEM_UNX
#elif defined(__linux__)
   #define SYSTEM_LINUX_64
   #define SYSTEM_UNX
#else
   #define SYSTEM_ERROR
#endif

#ifdef __x86_64__
   #define X86_64_CPU
#endif

#if defined(SYSTEM_UNX)
	#define CL_RESET  "\x1b[0m"
	#define CL_RED    "\x1b[31m"
	#define CL_BLUE   "\x1b[34m"
	#define CL_GREEN  "\x1b[32m"
	#define CL_YELLOW "\x1b[33m"
#elif defined(SYSTEM_WIN_64)
   #include <windows.h>
   #define CL_RESET  7
   #define CL_RED    4
   #define CL_BLUE   1	
   #define CL_GREEN  2
   #define CL_YELLOW 6
#endif

/*** Memory Allocation / GC ***/

/* OS MALLOC Function */
#define OPP_OS_MALLOC malloc

/* OS REALLOC Function */
#define OPP_OS_REALLOC realloc

/* OS FREE Function */
#define OPP_OS_FREE free

#define OPP_INITIAL_HEAP_SIZE (1024 * 6)

/* Debug Memory mode */
// #define DEBUG_MEMORY

/*** Lexer ***/

/* Size of buffer allocated for storing strings found while creating 
   tokens. If a string that is longer then this amount is detecting
   a reallocation is preformed adding this value to itself as the new 
   buffer size.
*/
#define SCAN_BUFFER_INITAL 64
_Static_assert(
   SCAN_BUFFER_INITAL >= 32, 
   "Scan Buffer Size needs to be at least 32 bytes"
);

#define MAX_IDENTIFIER_LEN 32

/*** Parser ***/

/* Size of default amount of space allocated for parser generated bytecode. 
   This value also represents the incrementation amount 
   when needed to preform a reallocation.
*/ 
#define PARSER_BYTECODE_SIZE 64

/* Size of global hashmap */
#define GLOBAL_HASHMAP_SIZE 64

#define LOCAL_LIST_SIZE 256

#endif