#include "../../../tools/raylib_port/opp_raylib.h"
#include "../parser/ostdlib.h"

#define ADD_FUNC(a,b) { if (!env_new_cfn(global_ns->inside, a, b)) internal_error("Raylib init fail", 2); }
#define ADD_VAR(a,b) {env_new_int(global_ns->inside, a, b); }

void init_raylib() 
{
	ADD_VAR("KEY_UP", 265);
	ADD_VAR("KEY_DOWN", 264);
	ADD_VAR("KEY_RIGHT", 262);
	ADD_VAR("KEY_LEFT", 263);

	ADD_FUNC("init_window", opp_InitWindow);
	ADD_FUNC("set_target_fps", opp_SetTargetFPS);
	ADD_FUNC("window_should_close", opp_WindowShouldClose);
	ADD_FUNC("begin_drawing", opp_BeginDrawing);
	ADD_FUNC("clear_background", opp_ClearBackground);
	ADD_FUNC("draw_text", opp_DrawText);
	ADD_FUNC("end_drawing", opp_EndDrawing);
	ADD_FUNC("close_window", opp_CloseWindow);

	ADD_FUNC("is_key_down", opp_isKeyDown);
	ADD_FUNC("draw_circleV", opp_DrawCircleV);
}