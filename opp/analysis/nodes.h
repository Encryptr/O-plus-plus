/** @file nodes.h
 * 
 * @brief Analysis nodes
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

#ifndef OPP_NODES
#define OPP_NODES

enum Analyzer_Instr {
	IR_BIN, 
	IR_LOGIC,
	IR_CALL,
	IR_ASSIGN,
	IR_UNARY, 
	IR_SUB,
	IR_ELEMENT,
	IR_DEREF,
	IR_ADJUST,
	IR_ADDR,

	IR_LABEL, 
	IR_GOTO, 
	IR_IF,
	IR_VAR,
	IR_IMPORT, 
	IR_WHILE,
	IR_FUNC, 
	IR_RET, 
	IR_EXTERN, 
	IR_FOR,
	IR_SWITCH,
	IR_CASE, 
	IR_BREAK,
};

struct Analyzer_Node;

struct Analyzer_List {
	struct Analyzer_Node** list;
	unsigned int len;
};

struct Analyzer_Unary {
	enum Opp_Token type;
	unsigned int loc;
	union {
		int64_t i64val;
		double  f64val;
		char*   strval;
	};
};

struct Analyzer_Bin {
	enum Opp_Token tok;
	struct Analyzer_Node* left;
	struct Analyzer_Node* right;
};

struct Analyzer_Assign {
	enum Opp_Token op;
	struct Analyzer_Node* val;
	struct Analyzer_Node* ident;
};

struct Analyzer_Var {
	struct Opp_Type_Decl type;
	struct Analyzer_List vars;
};

struct Analyzer_Func {
	unsigned int loc;
	char* name;
	struct Analyzer_Node** body;
};

struct Analyzer_Node {
	enum Analyzer_Instr op;
	union {
		struct Analyzer_Unary   op_unary;
		struct Analyzer_Bin 	op_bin;
		struct Analyzer_Assign  op_assign;

		struct Analyzer_Func    op_func;
	};
};

#endif /* OPP_NODES */