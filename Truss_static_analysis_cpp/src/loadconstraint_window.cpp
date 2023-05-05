#include "loadconstraint_window.h"

loadconstraint_window::loadconstraint_window()
	: is_show_window(false), is_add_constraint(false), is_add_load(false),
	constraint_angleDegrees(0.0f), load_angleDegrees(0.0f), selectedOptionIndex(0), loadValue(0),
	square_size(50.0f)
{
	// Empty constructor

}

loadconstraint_window::~loadconstraint_window()
{
	// Destructor
}

void loadconstraint_window::render_window()
{
	if (is_show_window == false)
		return;
	//ImGui::Begin("My Window");
	//ImGui::Text("Hello, world!");
	//if (ImGui::Button("Exit"))
	//{
	//    show_window = false;
	//}
	// ImGui::End();
	ImGui::Begin("Constraint/ Load");

	// Define an array of tab names
	const char* tab_names[] = { "Constraint", "Load" };

	// Set the current active tab index
	static int active_tab = 0;

	// Create the tab bar
	if (ImGui::BeginTabBar("MyTabBar"))
	{
		// Begin the first tab item
		if (ImGui::BeginTabItem(tab_names[0]))
		{
			// Reset the add load option
			is_add_load = false;

			// Set the active tab index
			active_tab = 0;

			// Add your constraint input controls here
			// Option to select the types of support
			// Define an array of options
			const int options_count = 2;
			const char* options[] = { "Pinned support", "Roller support", };

			// Define a string to hold the label for the popup select button
			std::string popupLabel = "Support: ";

			if (ImGui::Button((popupLabel + options[selectedOptionIndex]).c_str())) {
				ImGui::OpenPopup("Select an option");
			}

			if (ImGui::BeginPopup("Select an option")) {
				ImGui::Text("- Constraint Type -");
				ImGui::Separator();

				for (int i = 0; i < options_count; i++) {
					if (ImGui::Selectable(options[i], selectedOptionIndex == i)) {
						selectedOptionIndex = i;
					}
				}

				ImGui::EndPopup();
			}
			//_________________________________________________________________________________________



			// Input box to give input via text
			static bool input_mode = false;
			static char angle_str[4] = ""; // buffer to store input angle string
			static float angle_input = constraint_angleDegrees; // buffer to store input angle value

			// Button to switch to input mode
			if (!input_mode)
			{
				if (ImGui::Button("Input Angle"))
				{
					input_mode = true;
					snprintf(angle_str, 4, "%.1f", constraint_angleDegrees); // set the buffer to current angle
				}
			}
			else // input mode
			{
				// Text box to input angle value
				ImGui::SetNextItemWidth(60.0f);
				if (ImGui::InputText("##InputAngle", angle_str, IM_ARRAYSIZE(angle_str), ImGuiInputTextFlags_CharsDecimal))
				{
					// convert the input string to float
					angle_input = atof(angle_str);
					// limit the value to 0 - 360 range
					angle_input = fmaxf(0.0f, fminf(angle_input, 360.0f));
					// set the angle to input value
					constraint_angleDegrees = angle_input;
				}

				// Button to switch back to slider mode
				ImGui::SameLine();
				if (ImGui::Button("OK"))
				{
					input_mode = false;
				}
			}

			// Slider for angle
			ImGui::Text("Angle");
			ImGui::SameLine();
			ImGui::SliderFloat("Degrees", &constraint_angleDegrees, 0.0f, 360.0f, "%.1f");
			//_________________________________________________________________________________________

			// add some vertical spacing
			ImGui::Spacing();

			// Add Constraint
			if (is_add_constraint == true)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.8f, 0.4f, 1.0f)); // brighter green color
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // default color
			}

			if (ImGui::Button("Add Constraint"))
			{
				is_add_constraint = !is_add_constraint;
			}

			// Text to inform user whether Add constrain is added or not
			ImGui::SameLine(); // move cursor to the same line
			if (is_add_constraint == true)
			{
				ImGui::Text("Click on the nodes to add constraint");
			}
			else
			{
				ImGui::Text("<= Click here to start adding constraint");
			}

			ImGui::PopStyleColor(1);  // Revert back to default style

			// Close button
			if (ImGui::Button("Close"))
			{
				is_show_window = false; // set the flag to close the window
			}


			// Draw the rectangle
			ImVec2 window_pos = ImGui::GetCursorScreenPos();
			ImVec2 window_size = ImGui::GetContentRegionAvail();
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			draw_list->AddRect(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y), ImColor(255, 255, 255));

			// End the tab item
			ImGui::EndTabItem();
		}

		// Begin the second tab item
		if (ImGui::BeginTabItem(tab_names[1]))
		{
			// Reset the add constraint option
			is_add_constraint = false;

			// Set the active tab index
			active_tab = 1;

			// Input box to give input via text
			static bool loadval_input_mode = false;
			static char load_str[16] = ""; // buffer to store input load string
			static int load_input = 0; // buffer to store input load value

			// Button to switch to input mode
			if (!loadval_input_mode)
			{
				if (ImGui::Button("Input Load"))
				{
					loadval_input_mode = true;
					snprintf(load_str, 16, "%d", loadValue); // set the buffer to current load value
				}
			}
			else // input mode
			{
				// Text box to input load value
				ImGui::SetNextItemWidth(60.0f);
				if (ImGui::InputText("##InputLoad", load_str, IM_ARRAYSIZE(load_str), ImGuiInputTextFlags_CharsDecimal))
				{
					// convert the input string to int
					load_input = atoi(load_str);
					// set the load value to input value
					loadValue = load_input;
				}

				// Button to switch back to slider mode
				ImGui::SameLine();
				if (ImGui::Button("OK"))
				{
					loadval_input_mode = false;
				}
			}

			// Text for load value
			ImGui::SameLine();
			ImGui::Text("Load = %d", loadValue);
			//_________________________________________________________________________________________

			// Input box to give input via text
			static bool loadangle_input_mode = false;
			static char load_angle_str[4] = ""; // buffer to store input angle string
			static float load_angle_input = load_angleDegrees; // buffer to store input angle value

			// Button to switch to input mode
			if (!loadangle_input_mode)
			{
				if (ImGui::Button("Input Angle"))
				{
					loadangle_input_mode = true;
					snprintf(load_angle_str, 4, "%.1f", load_angleDegrees); // set the buffer to current angle
				}
			}
			else // input mode
			{
				// Text box to input angle value
				ImGui::SetNextItemWidth(60.0f);
				if (ImGui::InputText("##InputAngle", load_angle_str, IM_ARRAYSIZE(load_angle_str), ImGuiInputTextFlags_CharsDecimal))
				{
					// convert the input string to float
					load_angle_input = atof(load_angle_str);
					// limit the value to 0 - 360 range
					load_angle_input = fmaxf(0.0f, fminf(load_angle_input, 360.0f));
					// set the angle to input value
					load_angleDegrees = load_angle_input;
				}

				// Button to switch back to slider mode
				ImGui::SameLine();
				if (ImGui::Button("OK"))
				{
					loadangle_input_mode = false;
				}
			}

			// Slider for angle
			ImGui::Text("Angle");
			ImGui::SameLine();
			ImGui::SliderFloat("Degrees", &load_angleDegrees, 0.0f, 360.0f, "%.1f");
			//_________________________________________________________________________________________

				// add some vertical spacing
			ImGui::Spacing();

			// Add Constraint
			if (is_add_load == true)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.8f, 0.4f, 1.0f)); // brighter green color
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // default color
			}

			if (ImGui::Button("Add Load"))
			{
				is_add_load = !is_add_load;
			}

			// Text to inform user whether Add constrain is added or not
			ImGui::SameLine(); // move cursor to the same line
			if (is_add_load == true)
			{
				ImGui::Text("Click on the nodes to add load");
			}
			else
			{
				ImGui::Text("<= Click here to start adding load");
			}

			ImGui::PopStyleColor(1);  // Revert back to default style

			// Close button
			if (ImGui::Button("Close"))
			{
				is_show_window = false; // set the flag to close the window
			}


			// Draw the rectangle
			ImVec2 window_pos = ImGui::GetCursorScreenPos();
			ImVec2 window_size = ImGui::GetContentRegionAvail();
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			draw_list->AddRect(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y), ImColor(255, 255, 255));


			// End the tab item
			ImGui::EndTabItem();
		}

		// End the tab bar
		ImGui::EndTabBar();
	}
	ImGui::End();
}


