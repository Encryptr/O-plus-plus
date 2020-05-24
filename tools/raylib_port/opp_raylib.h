#include "../../Opp/src/parser/ostdlib.h"
#include <raylib.h>

OPP_MODULE opp_InitWindow(OPP_ARGS) {
	Opp_Return ret = Return_Type(OBJ_NONE);
	expect_args(3);

	Arg_Val val1 = opp_eval_expr(args->list[0]);
	Arg_Val val2 = opp_eval_expr(args->list[1]);
	Arg_Val val3 = opp_eval_expr(args->list[2]);
	
	InitWindow(val1->oint, val2->oint, val3->ostr);
	
	return ret;
}

OPP_MODULE opp_SetTargetFPS(OPP_ARGS) {
	Opp_Return ret = Return_Type(OBJ_NONE);
	expect_args(1);

	Arg_Val val1 = opp_eval_expr(args->list[0]);

	SetTargetFPS(val1->oint);
	
	return ret;
}

OPP_MODULE opp_WindowShouldClose(OPP_ARGS) {
	Opp_Return ret = Return_Type(OBJ_BOOL);
	expect_args(0);

	ret->obool = !WindowShouldClose();
	
	return ret;
}

OPP_MODULE opp_BeginDrawing(OPP_ARGS) {
	Opp_Return ret = Return_Type(OBJ_NONE);
	expect_args(0);

	BeginDrawing();
	
	return ret;
}

OPP_MODULE opp_ClearBackground(OPP_ARGS) {
	Opp_Return ret = Return_Type(OBJ_NONE);
	expect_args(0);

	// Arg_Val val1 = opp_eval_expr(args->list[0]);

	// Temp solution
	ClearBackground(RAYWHITE);
	
	return ret;
}

OPP_MODULE opp_DrawText(OPP_ARGS) {
	Opp_Return ret = Return_Type(OBJ_NONE);
	expect_args(4);

	Arg_Val val1 = opp_eval_expr(args->list[0]);
	Arg_Val val2 = opp_eval_expr(args->list[1]);
	Arg_Val val3 = opp_eval_expr(args->list[2]);
	Arg_Val val4 = opp_eval_expr(args->list[3]);
	// Arg_Val val5 = opp_eval_expr(args->list[4]);

	DrawText(val1->ostr, val2->oint, val3->oint, 
		val4->oint, LIGHTGRAY);
	
	return ret;
}

OPP_MODULE opp_EndDrawing(OPP_ARGS) {
	Opp_Return ret = Return_Type(OBJ_NONE);
	expect_args(0);

	EndDrawing();
	
	return ret;
}

OPP_MODULE opp_CloseWindow(OPP_ARGS) {
	Opp_Return ret = Return_Type(OBJ_NONE);
	expect_args(0);

	CloseWindow();
	
	return ret;
}

OPP_MODULE opp_isKeyDown(OPP_ARGS) {
	Opp_Return ret = Return_Type(OBJ_BOOL);
	expect_args(1);

	Arg_Val val1 = opp_eval_expr(args->list[0]);
	ret->obool = IsKeyDown(val1->oint);
	
	return ret;
}

// Raylib vector will be 2 separate variables
OPP_MODULE opp_DrawCircleV(OPP_ARGS) {
	Opp_Return ret = Return_Type(OBJ_NONE);
	expect_args(3);

	Arg_Val val1 = opp_eval_expr(args->list[0]);
	Arg_Val val2 = opp_eval_expr(args->list[1]);
	Arg_Val val3 = opp_eval_expr(args->list[2]);
	
	Vector2 vec = {val1->ofloat, val2->ofloat};

	DrawCircleV(vec, val3->oint, MAROON);
	
	return ret;
}
