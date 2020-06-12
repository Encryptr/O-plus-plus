#include "../../Opp/src/parser/ostdlib.h"
#include <raylib.h>

OPP_MODULE opp_InitWindow(OPP_ARGS, OPP_RETURN) {
	obj->obj_type = OBJ_NONE;
	expect_args(3);

	Arg_Val val1, val2, val3; 
	opp_eval_expr(args->list[0], &val1);
	opp_eval_expr(args->list[1], &val2);
	opp_eval_expr(args->list[2], &val3);

	InitWindow(val1.oint, val2.oint, val3.ostr);
}

OPP_MODULE opp_SetTargetFPS(OPP_ARGS, OPP_RETURN) {
	obj->obj_type = OBJ_NONE;
	expect_args(1);

	Arg_Val val1;
	opp_eval_expr(args->list[0], &val1);

	SetTargetFPS(val1.oint);
}

OPP_MODULE opp_WindowShouldClose(OPP_ARGS, OPP_RETURN) {
	obj->obj_type = OBJ_BOOL;
	expect_args(0);

	obj->obool = !WindowShouldClose();
}

OPP_MODULE opp_BeginDrawing(OPP_ARGS, OPP_RETURN) {
	obj->obj_type = OBJ_NONE;
	expect_args(0);

	BeginDrawing();
}

OPP_MODULE opp_ClearBackground(OPP_ARGS, OPP_RETURN) {
	obj->obj_type = OBJ_NONE;
	expect_args(0);

	// Temp solution
	ClearBackground(RAYWHITE);
}

OPP_MODULE opp_DrawText(OPP_ARGS, OPP_RETURN) {
	obj->obj_type = OBJ_NONE;
	expect_args(4);

	Arg_Val val1, val2, val3, val4;
	opp_eval_expr(args->list[0], &val1);
	opp_eval_expr(args->list[1], &val2);
	opp_eval_expr(args->list[2], &val3);
	opp_eval_expr(args->list[3], &val4);

	DrawText(val1.ostr, val2.oint, val3.oint, 
		val4.oint, LIGHTGRAY);
	
}

OPP_MODULE opp_EndDrawing(OPP_ARGS, OPP_RETURN) {
	obj->obj_type = OBJ_NONE;
	expect_args(0);

	EndDrawing();
}

OPP_MODULE opp_CloseWindow(OPP_ARGS, OPP_RETURN) {
	obj->obj_type = OBJ_NONE;
	expect_args(0);

	CloseWindow();
}

OPP_MODULE opp_isKeyDown(OPP_ARGS, OPP_RETURN) {
	obj->obj_type = OBJ_BOOL;
	expect_args(1);

	Arg_Val val1;
	opp_eval_expr(args->list[0], &val1);
	obj->obool = IsKeyDown(val1.oint);
}

// Raylib vector will be 2 separate variables
OPP_MODULE opp_DrawCircleV(OPP_ARGS, OPP_RETURN) {
	obj->obj_type = OBJ_NONE;
	expect_args(3);

	Arg_Val val1, val2, val3;
	opp_eval_expr(args->list[0], &val1);
	opp_eval_expr(args->list[1], &val2);
	opp_eval_expr(args->list[2], &val3);

	
	Vector2 vec = {val1.ofloat, val2.ofloat};

	DrawCircleV(vec, val3.oint, MAROON);
}