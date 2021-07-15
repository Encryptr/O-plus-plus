/** @file analizer.h
 * 
 * @brief Opp Analalysis Stage
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

#ifndef OPP_ANALIZER
#define OPP_ANALIZER

struct Debug_Info {
	char* fn_name;
	enum {
		SCOPE_GLOBAL,
		SCOPE_LOCAL,
		SCOPE_PARAM,
	} linkage;
};

struct Opp_Analyzer {
	struct Opp_Parser* parser;
	struct Opp_Hashmap* global, *scope;
	struct Debug_Info debug;
};

void opp_start_analyzer(struct Opp_Parser* parser);

#endif