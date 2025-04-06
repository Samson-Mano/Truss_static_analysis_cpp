#include "solver_window.h"

solver_window::solver_window()
{
	// Empty constructor
}

solver_window::~solver_window()
{
}

void solver_window::render_window()
{
	// Render the solver window
	if (is_show_window == false)
		return;

	ImGui::Begin("Finite Element Solver");

	// Option to select the types of solver
	// Define an array of options
	const int options_count = 3;
	const char* options[] = { "Penalty approach", "Lagrange approach", "Elimination approach"};

	// Define a string to hold the label for the popup select button
	std::string popupLabel = "Solver: ";

	if (ImGui::Button((popupLabel + options[solver_type]).c_str()))
	{
		ImGui::OpenPopup("Select an option");
	}

	if (ImGui::BeginPopup("Select an option")) {
		ImGui::Text("- Solver Type -");
		ImGui::Separator();

		for (int i = 0; i < options_count; i++) 
		{
			if (ImGui::Selectable(options[i], solver_type == i))
			{
				solver_type = i;
			}
		}

		ImGui::EndPopup();
	}
	//_________________________________________________________________________________________


	// Add a "Sovle" button
	if (ImGui::Button("FE Solve"))
	{
		execute_solver = true;
	}


	// Add a log text box
	ImGui::TextWrapped("Solver Log:");
	ImGui::BeginChild("log_scroll_region", ImVec2(-1, 200), true, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::TextUnformatted(log_buffer.c_str());

	// Automatic scroll to bottom
	if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		ImGui::SetScrollHereY(1.0f);

	ImGui::EndChild();


	ImGui::Spacing();
	//_________________________________________________________________________________________

	// Add check boxes to show the Deformed model
	ImGui::Checkbox("Show Model", &show_undeformed_model);
	ImGui::Checkbox("Show Result values", &show_result_text_values);
	ImGui::Checkbox("Show Reaction Force", &show_reactionforce);

	ImGui::Spacing();

	// Input box to give input via text
	static bool defscale_input_mode = false;
	static char defscale_str[16] = ""; // buffer to store input deformation scale string
	static float defscale_input = 0; // buffer to store input deformation scale value

	// Button to switch to input mode
	if (!defscale_input_mode)
	{
		if (ImGui::Button("Deformation Scale"))
		{
			defscale_input_mode = true;
			snprintf(defscale_str, 16, "%.1f", deformation_scale_max); // set the buffer to current deformation scale value
		}
	}
	else // input mode
	{
		// Text box to input value
		ImGui::SetNextItemWidth(60.0f);
		if (ImGui::InputText("##Deformation Scale", defscale_str, IM_ARRAYSIZE(defscale_str), ImGuiInputTextFlags_CharsDecimal))
		{
			// convert the input string to int
			defscale_input = atoi(defscale_str);
			// set the load value to input value
			deformation_scale_max = defscale_input;
		}

		// Button to switch back to slider mode
		ImGui::SameLine();
		if (ImGui::Button("OK"))
		{
			defscale_input_mode = false;
		}
	}

	// Text for load value
	ImGui::SameLine();
	ImGui::Text(" %.1f", deformation_scale_max);

	// Slider for angle
	float deformation_scale_flt = deformation_scale_max;

	ImGui::Text("Deformation Scale");
	ImGui::SameLine();
	ImGui::SliderFloat(".", &deformation_scale_flt, 0.0f, 100.0f, "%.1f");
	deformation_scale_max = deformation_scale_flt;

	//Set the deformation scale
	deformation_scale = deformation_scale_max;

	ImGui::Spacing();
	//_________________________________________________________________________________________

	if (ImGui::CollapsingHeader("Animate"))
	{
		// Animate the solution
		// Start a horizontal layout
		ImGui::BeginGroup();

		// Play button active
		if (animate_play == true)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.8f, 0.4f, 1.0f)); // brighter green color
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // default color
		}

		// Add the Play button
		if (ImGui::Button("Play"))
		{
			// Handle Play button click
			animate_play = !animate_play;
			animate_pause = false;
		}
		ImGui::PopStyleColor(1);  // Revert back to default style

		// Add some spacing between buttons
		ImGui::SameLine();


		// Pause button active
		if (animate_pause == true)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.8f, 0.4f, 1.0f)); // brighter green color
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // default color
		}

		// Add the Pause button
		if (ImGui::Button("Pause"))
		{
			// Handle Pause button click
			animate_pause = !animate_pause;
			animate_play = false;
		}
		ImGui::PopStyleColor(1);  // Revert back to default style

		// Add some spacing between buttons
		ImGui::SameLine();

		// Add the Stop button
		if (ImGui::Button("Stop"))
		{
			// Handle Stop button click
			animate_play = false;
			animate_pause = false;
			time_val = 0.0f;
		}

		// Animation speed control
		// Input box to give input via text
		static bool animation_speed_input_mode = false;
		static char animation_speed_str[16] = ""; // buffer to store input deformation scale string
		static float animation_speed_input = 0; // buffer to store input deformation scale value

		// Button to switch to input mode
		if (!animation_speed_input_mode)
		{
			if (ImGui::Button("Animation Speed"))
			{
				animation_speed_input_mode = true;
				snprintf(animation_speed_str, 16, "%.1f", animation_speed); // set the buffer to current deformation scale value
			}
		}
		else // input mode
		{
			// Text box to input value
			ImGui::SetNextItemWidth(60.0f);
			if (ImGui::InputText("##Animation Speed", animation_speed_str, IM_ARRAYSIZE(animation_speed_str), ImGuiInputTextFlags_CharsDecimal))
			{
				// convert the input string to int
				animation_speed_input = atoi(animation_speed_str);
				// set the load value to input value
				animation_speed = animation_speed_input;
			}

			// Button to switch back to slider mode
			ImGui::SameLine();
			if (ImGui::Button("OK"))
			{
				animation_speed_input_mode = false;
			}
		}

		// Text for Animation speed value
		ImGui::SameLine();
		ImGui::Text(" %.1f", animation_speed);

		// Display the frame rate
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
			1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		// End the horizontal layout
		ImGui::EndGroup();
	}

	ImGui::Spacing();
	//_________________________________________________________________________________________


	if (ImGui::CollapsingHeader("Solution"))
	{
		// Add the radio buttons
		ImGui::RadioButton("Deflection", &selected_solution_option, 0);
		ImGui::RadioButton("Member Forces", &selected_solution_option, 1);
		ImGui::RadioButton("Member Stress", &selected_solution_option, 2);
	}

	ImGui::Spacing();
	//_________________________________________________________________________________________

		// Close button
	if (ImGui::Button("Close"))
	{
		log_buffer = "";
		execute_close = true;
		execute_open = false;
		is_show_window = false; // set the flag to close the window
	}

	ImGui::End();


	// Set the animation data
	if (animate_play == true)
	{
		if (time_val > 6.283185307f)
		{
			time_val = 0.0f;
		}

		// Animation is playing 
		deformation_scale = ((std::cos(time_val * animation_speed) + 1) * 0.5f) * deformation_scale_max;
		time_val = time_val + 0.0002f;
	}
	else if (animate_pause == true)
	{
		deformation_scale = ((std::cos(time_val * animation_speed) + 1) * 0.5f) * deformation_scale_max;
	}
}

void solver_window::reset_solver_window()
{
	// reset the solver window
	is_analysis_complete = false; // Track the run status
	is_show_window = false;

	execute_open = false; // Solver window open event flag
	execute_solver = false; // Main solver run event flag
	execute_close = false; // Closing of solution window event flag
	show_undeformed_model = true; // show undeformed model 
	show_reactionforce = true; // show the results on deformed
	show_result_text_values = true; // show the result text values
	animate_play = false;
	animate_pause = false;
	selected_solution_option = 0;
	solver_type = 0;

}


