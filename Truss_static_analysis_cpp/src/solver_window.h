#pragma once
#include <iostream>
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"

class solver_window
{
public:
	bool is_analysis_complete = false; // Track the run status
	bool is_show_window = false;

	bool execute_open = false; // Solver window open event flag
	bool execute_solver = false; // Main solver run event flag
	bool execute_close = false; // Closing of solution window event flag
	bool show_undeformed_model = true; // show undeformed model 
	bool show_reactionforce = true; // show the results on deformed
	bool show_result_text_values = true; // show the result text values
	bool animate_play = false;
	bool animate_pause = false;
	int selected_solution_option = 0;
	float time_val = 0.0f;
	double deformation_scale_max = 30.0f;
	double animation_speed = 10.0f;
	double deformation_scale = 0.0;

	int solver_type = 0;

	std::string log_buffer;

	solver_window();
	~solver_window();
	void render_window();
	void reset_solver_window();
private:

};