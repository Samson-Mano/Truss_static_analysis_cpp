#include "loadconstraint_window.h"

loadconstraint_window::loadconstraint_window()
	: is_show_window(false), is_add_constraint(false), is_add_load(false),
	constraint_angleDegrees(90.0f), load_angleDegrees(0.0f), constraint_selectedOptionIndex(0), loadValue(100.0f),
	square_size(50.0f)
{

}

loadconstraint_window::~loadconstraint_window()
{
	// Destructor
}

void loadconstraint_window::bind_images()
{
	// Load the texture from a file
	//std::filesystem::path currentDirPath = std::filesystem::current_path();
	//std::filesystem::path parentPath = currentDirPath.parent_path();
	//std::filesystem::path shadersPath = parentPath / "Truss_static_analysis_cpp/src/geometry_store/shaders";

	std::string img_path = "";
	unsigned int my_image_texture = 0;

	// Bind the pin image
	img_path = "src/geometry_store/shaders/pic_pin_support.png";
	LoadTextureFromFile(img_path.c_str(), &my_image_texture, pin_image.is_loaded);

	pin_image.image_texture = my_image_texture;

	// Bind the roller image
	img_path = "src/geometry_store/shaders/pic_roller_support.png";
	LoadTextureFromFile(img_path.c_str(), &my_image_texture, roller_image.is_loaded);

	roller_image.image_texture = my_image_texture;

	// Bind the Load image
	img_path = "src/geometry_store/shaders/pic_load_img.png";
	LoadTextureFromFile(img_path.c_str(), &my_image_texture, load_image.is_loaded);

	load_image.image_texture = my_image_texture;
}

void loadconstraint_window::render_window()
{
	if (is_show_window == false)
		return;

	ImGui::Begin("Constraint/ Load");

	// Define an array of tab names
	const char* tab_names[] = { "Constraint", "Load" };

	// Set the current active tab index
	static int active_tab = 0;

	// Create the tab bar
	if (ImGui::BeginTabBar("MyTabBar"))
	{
		// Render the first tab item - Constraint Tab
		if (ImGui::BeginTabItem(tab_names[0]))
		{
			// Render constraint tab
			render_constraint_tab();
		}

		// Render the second tab item - Add Tab
		if (ImGui::BeginTabItem(tab_names[1]))
		{
			// Render load tab
			render_load_tab();
		}

		// End the tab bar
		ImGui::EndTabBar();
	}
	ImGui::End();
}

void loadconstraint_window::render_constraint_tab()
{
	// Render the Constraint Tab
				// Reset the add load option
	is_add_load = false;

	// Set the active tab index
	static int active_tab = 0;

	// Add your constraint input controls here
	// Option to select the types of support
	// Define an array of options
	const int options_count = 2;
	const char* options[] = { "Pinned support", "Roller support", };

	// Define a string to hold the label for the popup select button
	std::string popupLabel = "Support: ";

	if (ImGui::Button((popupLabel + options[constraint_selectedOptionIndex]).c_str())) {
		ImGui::OpenPopup("Select an option");
	}

	if (ImGui::BeginPopup("Select an option")) {
		ImGui::Text("- Constraint Type -");
		ImGui::Separator();

		for (int i = 0; i < options_count; i++) {
			if (ImGui::Selectable(options[i], constraint_selectedOptionIndex == i)) {
				constraint_selectedOptionIndex = i;
			}
		}

		ImGui::EndPopup();
	}
	//_________________________________________________________________________________________

	// Input box to give input via text
	static bool input_mode = false;
	static char angle_str[16] = ""; // buffer to store input angle string
	static float angle_input = constraint_angleDegrees; // buffer to store input angle value

	// Button to switch to input mode
	if (!input_mode)
	{
		if (ImGui::Button("Input Angle"))
		{
			input_mode = true;
			snprintf(angle_str, 16, "%.1f", constraint_angleDegrees); // set the buffer to current angle
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
	
	//_________________________________________________________________________________________
	

	// Close button
	if (ImGui::Button("Close"))
	{
		is_add_constraint = false;
		is_add_load = false;
		is_show_window = false; // set the flag to close the window
	}
	//_________________________________________________________________________________________
	

	// Render reference image
	if (constraint_selectedOptionIndex == 0)
	{
		// Draw pin support
		draw_pin_support();
	}
	else
	{
		// Draw roller support
		draw_roller_support();
	}

	// End the tab item
	ImGui::EndTabItem();
}

void loadconstraint_window::render_load_tab()
{
	// Render the Load Tab
			// Reset the add constraint option
	is_add_constraint = false;

	// Set the active tab index
	static int active_tab = 1;

	// Input box to give input via text
	static bool loadval_input_mode = false;
	static char load_str[16] = ""; // buffer to store input load string
	static float load_input = 0; // buffer to store input load value

	// Button to switch to input mode
	if (!loadval_input_mode)
	{
		if (ImGui::Button("Input Load"))
		{
			loadval_input_mode = true;
			snprintf(load_str, 16, "%.1f", loadValue); // set the buffer to current load value
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
	ImGui::Text("Load = %.1f", loadValue);
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
		is_add_constraint = false;
		is_add_load = false;
		is_show_window = false; // set the flag to close the window
	}

	// Draw the reference Image
	draw_load_image();

	// End the tab item
	ImGui::EndTabItem();
}

void loadconstraint_window::draw_pin_support()
{
	// Draw the pin support image
	if (pin_image.is_loaded == true)
	{
		ImVec2 window_pos = ImGui::GetCursorScreenPos();
		ImVec2 window_size = ImGui::GetContentRegionAvail();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();


		draw_list->AddRect(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y), ImColor(255, 255, 255));

		ImVec2 img_pos_top_left(0, 0);
		ImVec2 img_pos_top_right(0, 0);
		ImVec2 img_pos_bot_right(0, 0);
		ImVec2 img_pos_bot_left(0, 0);

		bool draw_img = get_image_min_max_coord(window_pos, window_size, img_pos_top_left, img_pos_top_right, img_pos_bot_right, img_pos_bot_left, constraint_angleDegrees);

		if (draw_img == true)
		{
			draw_list->AddImageQuad((void*)(intptr_t)pin_image.image_texture, img_pos_top_left, img_pos_top_right, img_pos_bot_right, img_pos_bot_left);
		}
	}
}

void loadconstraint_window::draw_roller_support()
{
	// Draw the roller support image
	if (roller_image.is_loaded == true)
	{
		ImVec2 window_pos = ImGui::GetCursorScreenPos();
		ImVec2 window_size = ImGui::GetContentRegionAvail();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();


		draw_list->AddRect(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y), ImColor(255, 255, 255));

		ImVec2 img_pos_top_left(0, 0);
		ImVec2 img_pos_top_right(0, 0);
		ImVec2 img_pos_bot_right(0, 0);
		ImVec2 img_pos_bot_left(0, 0);

		bool draw_img = get_image_min_max_coord(window_pos, window_size, img_pos_top_left, img_pos_top_right, img_pos_bot_right, img_pos_bot_left, constraint_angleDegrees);

		if (draw_img == true)
		{
			draw_list->AddImageQuad((void*)(intptr_t)roller_image.image_texture, img_pos_top_left, img_pos_top_right, img_pos_bot_right, img_pos_bot_left);
		}

	}
}

void loadconstraint_window::draw_load_image()
{
	// Draw the roller support image
	if (load_image.is_loaded == true)
	{
		ImVec2 window_pos = ImGui::GetCursorScreenPos();
		ImVec2 window_size = ImGui::GetContentRegionAvail();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();


		draw_list->AddRect(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y), ImColor(255, 255, 255));

		ImVec2 img_pos_top_left(0, 0);
		ImVec2 img_pos_top_right(0, 0);
		ImVec2 img_pos_bot_right(0, 0);
		ImVec2 img_pos_bot_left(0, 0);

		bool draw_img = get_image_min_max_coord(window_pos, window_size, img_pos_top_left, img_pos_top_right, img_pos_bot_right, img_pos_bot_left, load_angleDegrees);

		if (draw_img == true)
		{
			draw_list->AddImageQuad((void*)(intptr_t)load_image.image_texture, img_pos_top_left, img_pos_top_right, img_pos_bot_right, img_pos_bot_left);
		}
	}
}

bool loadconstraint_window::get_image_min_max_coord(ImVec2& window_pos, ImVec2& window_size, ImVec2& img_pos_top_left, ImVec2& img_pos_top_right, ImVec2& img_pos_bot_right, ImVec2& img_pos_bot_left, float& degrees)
{
	// Get the image position based on rotation angle
	if (window_size.x < 50 || window_size.y < 50)
		return false;

	// Rectangle origin
	glm::vec2 rect_window_origin = glm::vec2((window_pos.x + window_pos.x + window_size.x) * 0.5f,
		(window_pos.y + window_pos.y + window_size.y) * 0.5f);

	float rect_min_size = std::min(window_size.x, window_size.y);

	const float size_factor = 0.415f;

	// Corners of the image with origin (0,0)
	glm::vec2 top_left = glm::vec2(-(rect_min_size * size_factor), (rect_min_size * size_factor));
	glm::vec2 top_right = glm::vec2((rect_min_size * size_factor), (rect_min_size * size_factor));
	glm::vec2 bot_right = glm::vec2((rect_min_size * size_factor), -(rect_min_size * size_factor));
	glm::vec2 bot_left = glm::vec2(-(rect_min_size * size_factor), -(rect_min_size * size_factor));

	float radians = ((degrees + 90) * 3.14159365f) / 180.0f; // convert degrees to radians
	float cos_theta = cos(radians);
	float sin_theta = sin(radians);

	// Rotated point of the corners
	ImVec2 rotated_pt_top_left = ImVec2((top_left.x * cos_theta) - (top_left.y * sin_theta),
		(top_left.x * sin_theta) + (top_left.y * cos_theta));

	ImVec2 rotated_pt_top_right = ImVec2((top_right.x * cos_theta) - (top_right.y * sin_theta),
		(top_right.x * sin_theta) + (top_right.y * cos_theta));

	ImVec2 rotated_pt_bot_right = ImVec2((bot_right.x * cos_theta) - (bot_right.y * sin_theta),
		(bot_right.x * sin_theta) + (bot_right.y * cos_theta));

	ImVec2 rotated_pt_bot_left = ImVec2((bot_left.x * cos_theta) - (bot_left.y * sin_theta),
		(bot_left.x * sin_theta) + (bot_left.y * cos_theta));

	// Set the corner points after rotating and translating with the origin
	img_pos_top_left = ImVec2(rect_window_origin.x + rotated_pt_top_left.x, rect_window_origin.y + rotated_pt_top_left.y);
	img_pos_top_right = ImVec2(rect_window_origin.x + rotated_pt_top_right.x, rect_window_origin.y + rotated_pt_top_right.y);
	img_pos_bot_right = ImVec2(rect_window_origin.x + rotated_pt_bot_right.x, rect_window_origin.y + rotated_pt_bot_right.y);
	img_pos_bot_left = ImVec2(rect_window_origin.x + rotated_pt_bot_left.x, rect_window_origin.y + rotated_pt_bot_left.y);

	return true;
}

void loadconstraint_window::LoadTextureFromFile(const char* filename, unsigned int* out_texture, bool& s_IsInitialized)
{
	// Simple helper function to load an image into a OpenGL texture with common settings
	// Load from file
	int image_width = 0;
	int image_height = 0;
	unsigned int s_TextureID = 0;

	// Load from file only once
	if (!s_IsInitialized)
	{
		// Load image data from file
		unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
		if (image_data == NULL)
		{
			s_IsInitialized = false;
			return;
		}

		// Create OpenGL texture
		glGenTextures(1, &s_TextureID);
		glBindTexture(GL_TEXTURE_2D, s_TextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
		stbi_image_free(image_data);

		s_IsInitialized = true;
	}

	// Return the texture ID, width, and height
	*out_texture = s_TextureID;
}