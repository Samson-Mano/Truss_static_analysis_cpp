#include "material_window.h"

material_window::material_window()
	: is_show_window(false)
{
	// Constructor
}

material_window::~material_window()
{
	// Destructor
}

void material_window::render_window()
{
	if (is_show_window == false)
		return;

	if (material_list.size() == 0)
	{
		is_show_window = false;
		return;
	}


	ImGui::Begin("Materials");

	// Convert material_list to a vector of const char*
	std::vector<const char*> material_names;
	std::unordered_map<int, int> material_id_selected_option;
	int i = 0;

	for (const auto& mat : material_list)
	{
		material_id_selected_option[i] = mat.first;
		material_names.push_back(mat.second.material_name.c_str());
		i++;
	}

	ImGui::ListBox("Select Material", &selected_list_option, material_names.data(), material_names.size(), 4);

	ImGui::Spacing();

	selected_material_option = material_id_selected_option[selected_list_option];
	// Get selected material
	const material_data& selected_material_data = material_list[selected_material_option];

	// Get the color for this material
	glm::vec3 std_color = get_standard_color(selected_material_data.material_id);
	ImVec4 text_color = ImVec4(std_color.x, std_color.y, std_color.z, 1.0f);

	ImGui::TextColored(text_color, "Material ID: %i", selected_material_data.material_id);
	ImGui::TextColored(text_color, "Selected Material: %s", selected_material_data.material_name.c_str());
	ImGui::TextColored(text_color, "Young's Modulus: %.4e", selected_material_data.youngs_mod);
	ImGui::TextColored(text_color, "Density: %.4e", selected_material_data.mat_density);
	ImGui::TextColored(text_color, "Cross-Section Area: %.3f", selected_material_data.cs_area);

	// Diable delete if the selected option is Default (0)
	const bool is_delete_button_disabled = selected_list_option == 0 || selected_list_option == 1 ? true : false;
	ImGui::BeginDisabled(is_delete_button_disabled);
	if (ImGui::Button("Delete Material")) {
		// Delete material
		execute_delete_materialid = selected_material_data.material_id;
		material_list.erase(selected_material_data.material_id);
		selected_list_option = 0;
	}
	ImGui::EndDisabled();

	// Add some spacing before the "Create Material" header
	ImGui::Spacing();

	// Assign material dropdown
	if (ImGui::CollapsingHeader("Assign Material "))
	{
		// Add Constraint
		if (is_assign_material == true)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.8f, 0.4f, 1.0f)); // brighter green color
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // default color
		}

		if (ImGui::Button("Assign Material"))
		{
			is_assign_material = !is_assign_material;
		}

		// Text to inform user whether Add constrain is added or not
		ImGui::SameLine(); // move cursor to the same line
		if (is_assign_material == true)
		{
			ImGui::Text("Click on the member to assign the selected material");
		}
		else
		{
			ImGui::Text("<= Click here to start assign material");
		}

		ImGui::PopStyleColor(1); // Pop the style color after using it

		//// Display allocated members
		//ImGui::Text("Allocated Members:");
		//ImGui::BeginChild("AllocatedMembers", ImVec2(0, 100), true, ImGuiWindowFlags_HorizontalScrollbar);
		//for (const auto& member : selected_material_data.allocated_members)
		//{
		//	ImGui::TextUnformatted(member.c_str());
		//}
		//ImGui::EndChild();
	}

	ImGui::Spacing();

	// Create material dropdown
	if (ImGui::CollapsingHeader("Create Material "))
	{
		static char new_material_name[256] = "New Material";
		static double new_material_density = 7.865 * std::pow(10, -9);
		static double new_material_youngs_modulus = 210000.0;
		static double new_material_cs_area = 8000.0;

		ImGui::InputText("Material Name", new_material_name, IM_ARRAYSIZE(new_material_name));
		ImGui::InputDouble("Density", &new_material_density, 0, 0, "%.4e");
		ImGui::InputDouble("Young's Modulus", &new_material_youngs_modulus, 0, 0, "%.4e");
		ImGui::InputDouble("Cross-Section Area", &new_material_cs_area, 0, 0, "%.3f");

		if (ImGui::Button("Create Material"))
		{
			// TODO: Add the new material to the material list
			  // Add the new material to the material list
			material_data new_material;
			new_material.material_id = get_unique_material_id();
			new_material.material_name = new_material_name;
			new_material.mat_density = new_material_density;
			new_material.youngs_mod = new_material_youngs_modulus;
			new_material.cs_area = new_material_cs_area;
			material_list[new_material.material_id] = new_material;

			// Update the combo box
			selected_list_option = material_list.size() - 1;
		}
	}


	ImGui::Spacing();

	// Add a "Close" button
	if (ImGui::Button("Close"))
	{
		is_assign_material = false;
		is_show_window = false;
	}

	ImGui::End();
}

glm::vec3 material_window::get_standard_color(int color_index)
{
	// Red, Green, Blue, Yellow, Magenta, Cyan, Orange, Purple, Lime, Pink
	static const std::vector<glm::vec3> colorSet = {
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 0.0f),
			glm::vec3(1.0f, 0.0f, 1.0f),
			glm::vec3(0.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 0.5f, 0.0f),
			glm::vec3(0.5f, 0.0f, 1.0f),
			glm::vec3(0.5f, 1.0f, 0.0f),
			glm::vec3(1.0f, 0.0f, 0.5f)
	};

	int index = color_index % colorSet.size();
	return colorSet[index];
}

int material_window::get_unique_material_id()
{
	// Add all the ids to a int list
	std::vector<int> all_ids;
	for (auto& mat : material_list)
	{
		all_ids.push_back(mat.first);
	}


	if (all_ids.size() != 0)
	{
		int i;
		std::sort(all_ids.begin(), all_ids.end());

		// Find if any of the nodes are missing in an ordered int
		for (i = 0; i < all_ids.size(); i++)
		{
			if (all_ids[i] != i)
			{
				return i;
			}
		}

		// no node id is missing in an ordered list so add to the end
		return all_ids.size();
	}

	// id for the first node is 0
	return 0;
}
