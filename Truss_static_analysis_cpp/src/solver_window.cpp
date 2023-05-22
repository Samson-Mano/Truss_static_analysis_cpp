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


	// Add a "Sovle" button
	if (ImGui::Button("FE Solve"))
	{
		execute_solver = true;
	}

	// Add a log text box
	ImGui::TextWrapped("Solver Log:");

	

	constexpr size_t kLogBufferSize = 1024;
	char log_char[kLogBufferSize];

	// Copy the log string
	strcpy_s(log_char, kLogBufferSize, log_buffer.c_str());

	if (ImGui::InputTextMultiline("##log", log_char, kLogBufferSize, ImVec2(-1.0f, ImGui::GetTextLineHeight() * 10),
		ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_EnterReturnsTrue))
	{
		ImGui::SetScrollHereY(1.0f);
	}

	ImGui::Spacing();
	//_________________________________________________________________________________________

	// Add check boxes to show the Deformed model
	ImGui::Checkbox("Show Model", &show_undeformed_model);
	ImGui::Checkbox("Show Deformation", &show_deformed_model);
	ImGui::Checkbox("Show Result values", &show_result_text_values);

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
			snprintf(defscale_str, 16, "%.1f", deformation_scale); // set the buffer to current deformation scale value
		}
	}
	else // input mode
	{
		// Text box to input load value
		ImGui::SetNextItemWidth(60.0f);
		if (ImGui::InputText("##Deformation Scale", defscale_str, IM_ARRAYSIZE(defscale_str), ImGuiInputTextFlags_CharsDecimal))
		{
			// convert the input string to int
			defscale_input = atoi(defscale_str);
			// set the load value to input value
			deformation_scale = defscale_input;
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
	ImGui::Text("Deformation Scale = %.1f", deformation_scale);

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
		}

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





}



