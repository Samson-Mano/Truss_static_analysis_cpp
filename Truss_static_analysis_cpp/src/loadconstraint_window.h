#pragma once
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../ImGui/stb_implement.h"

class loadconstraint_window
{
public:
	loadconstraint_window();
	~loadconstraint_window();
	void render_window();
	bool is_show_window = false;
	bool is_add_constraint = false;
	bool is_add_load = false;
	int selectedOptionIndex = 0;
	float constraint_angleDegrees = 0.0f;
	float load_angleDegrees = 0.0f;
	float square_size = 50.0f;
	int loadValue = 0;
private:

};