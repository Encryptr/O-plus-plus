/** @file array.h
 * 
 * @brief Opp Dynamic Array
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

#ifndef OPP_ARRAY
#define OPP_ARRAY

#define ARRAY(T)                    \
    struct {                        \
        unsigned int capacity, len; \
        T* data;                    \
    }

#define new_array(arr, cap)                \
    do {                                   \
        arr.capacity = cap;                \
        arr.len = 0;                       \
        arr.data = opp_alloc(cap *         \
                    sizeof(*arr.data));    \
        if (!arr.data)                     \
            MALLOC_FAIL();                 \
    } while (0)


#endif /* OPP_ARRAY */