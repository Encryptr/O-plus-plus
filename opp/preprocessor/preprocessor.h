/** @file preprocessor.h
 * 
 * @brief Opp Preprocessor
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

#ifndef OPP_PREPROCESSOR
#define OPP_PREPROCESSOR

const char* opp_invoke_preproc(const char* path);
void opp_cleanup(const char* path);

#endif /* OPP_PREPROCESSOR */