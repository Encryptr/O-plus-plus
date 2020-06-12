#include "../Opp/src/parser/ostdlib.h"
#include "../Opp/src/env/enviroment.h"
/*

Example on how to make C lib ports to O++

*/

// Define the function like (provide your own name)
OPP_MODULE opp_example(OPP_ARGS, OPP_RETURN) {
	/* Define the functions return type */
	/* -- Options -- */
	// RETURN_NONE;
	RETURN_NUM;
	// RETURN_REAL;
	// RETURN_BOOL;
	// RETURN_STR;

	// Call and provide how many arguments the function expects
	expect_args(2);

	// Call to check types of provided arguments
	type_check( ((enum Opp_Obj_Type[2]){OBJ_INT, OBJ_INT}), args);

	// Define 'Arg_Val' so the paramaters value can be evaluated
	Arg_Val val1, val2;
	opp_eval_expr(args->list[0], &val1);
	opp_eval_expr(args->list[1], &val2);

	int sum = val1.oint + val2.oint;

	// Call return value and provide the value to return
	Return_Value(sum);
}

/*

	*** YOU MUST HAVE THIS FUNCTION 'opp_init_lib' ***
	Here you define all the functions you want to add to opp

*/

void opp_init_lib(OPP_ENV) 
{
	/* Call this and provide the name you will use to 
	call the function in O++ followed by the function 
	declared above 
	*/
	ADD_FUNC("example", opp_example);
}