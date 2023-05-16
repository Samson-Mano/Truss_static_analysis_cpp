#pragma once
#include <iostream>
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"

class options_window
{
public:
	bool is_show_nodenumber = true;
	bool is_show_nodecoord = false;
	bool is_show_linenumber = true;
	bool is_show_linelength = false;
	bool is_show_loadvalue = true;
	bool is_show_window = false;

	options_window();
	~options_window();
	void render_window();
private:

};
