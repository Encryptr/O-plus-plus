/** @file os.h
 * 
 * @brief File for all operating system header declarations 
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

#ifdef MINGW
#	define WIN64
#	define PLATFORM_NAME "windows"
#	include "win_header.h"
#endif

#ifdef __APPLE__
#	define MAC64
#	define UNX
#	define PLATFORM_NAME "macOS"
#	include "mac-header.h"
#endif

#ifdef __linux__
#	define LINUX64
#	define UNX
#	define PLATFORM_NAME "linux"
#	include "linux-header.h"
#endif

#ifdef __x86_64__
#	define X86_CPU
#endif

#if !defined(WIN) && !defined(UNX)
	#define OS_ERROR
#endif

#ifdef UNX
	#define CL_RED    "\x1b[31m"
	#define CL_BLUE   "\x1b[34m"
	#define CL_GREEN  "\x1b[32m"
	#define CL_RESET  "\x1b[0m"
	#define CL_YELLOW "\x1b[33m"
#endif

#endif /* OPP_HEADER */