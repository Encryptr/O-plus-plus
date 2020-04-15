#ifndef OPP_OBJ
#define OPP_OBJ

enum Opp_Type {
	T_INVALID, T_INTEGER, 
	T_REAL, T_STRING, T_ARRAY,
	T_END
};

struct Opp_Value {
	enum Opp_Type val_type;

	union {
		int ival;
		double real;
		char* strval;
	};

	struct Opp_Value* next; 
};

#endif 