/** @file ast.h
 * 
 * @brief Opp Ast
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

#ifndef OPP_AST
#define OPP_AST

#include "../lexer/lexer.h"

enum Opp_Node_Type {
    NODE_NONE,

	EXPR_BINARY, 
    EXPR_CALL,
    EXPR_UNARY, 
    EXPR_NEG,
	EXPR_INDEX,
	EXPR_DOT, 

	STMT_ASSIGN, 
	STMT_IF,
	STMT_BLOCK, 
    STMT_VAR,
	STMT_IMPORT, 
    STMT_WHILE,
	STMT_FUNC, 
    STMT_RET, 
    STMT_FOR,
    STMT_BREAK,
};

struct Opp_Node {
    enum Opp_Node_Type type;
    struct {
        unsigned int line, colum;
        char* loc;
    } debug;

    union {
        /* Includes:
            TGT, TLE, TLT, TGE,
            TEQEQ, TNOTEQ, TNOT,  
	        TADD, TMIN, TDIV, TMUL, 
            TMOD, TDOT, TEQ, TADDEQ, 
            TMINEQ, TDIVEQ, TMULEQ, TMODEQ,
        */
        struct {
	        struct Opp_Node* left;
            struct Opp_Node* right;
            enum Opp_Token operand;
        } bin_expr;

        /* Includes only function call expr */
        struct {
            struct Opp_Node* caller;
            // list of args
        } call_expr;

        /* Unary values:
            TIDENT, TINTEGER, TFLOATING, TSTR
        */
        struct {
            enum Opp_Token type;
            // union Opp_Value value;
        } unary_expr;

        struct {
            struct Opp_Node* expr;
            enum Opp_Token operand;
        } expr_atom;

        struct {
            struct Opp_Node* cond;
            struct Opp_Node* then;
            struct Opp_Node* other;
        } stmt_if;

        struct {
            //figure this out
            struct Opp_Node* t;
        } stmt_block;
    } node;
};


#endif /* OPP_AST */