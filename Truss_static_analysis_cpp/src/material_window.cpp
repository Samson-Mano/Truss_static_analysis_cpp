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
		return;

	ImGui::Begin("Materials");

	// Convert material_list to a vector of const char*
	std::vector<const char*> material_names;
	for (const auto& mat : material_list) {
		material_names.push_back(mat.material_name.c_str());
	}


	//// ImGui::Combo("Select Material", &selected_material_option, material_names.data(), material_names.size());
	ImGui::ListBox(".", &selected_material_option, material_names.data(), material_names.size(),4);

	ImGui::Spacing();
	// Get selected material
	const material_data& selected_material_data = material_list[selected_material_option];

	ImGui::Text("Selected Material: %s", selected_material_data.material_name.c_str());
	ImGui::Text("Young's Modulus: %.3f", selected_material_data.youngs_mod);
	ImGui::Text("Density: %.3e", selected_material_data.mat_density);
	ImGui::Text("Cross-Section Area: %.3f", selected_material_data.cs_area);

	// Diable delete if the selected option is Default (0)
	const bool is_delete_button_disabled = selected_material_option ==0?true:false;
	ImGui::BeginDisabled(is_delete_button_disabled);
	if (ImGui::Button("Delete Material")) {
		// Delete material
		material_list.erase(material_list.begin() + selected_material_option);
		selected_material_option = 0;
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
		ImGui::InputDouble("Density", &new_material_density,0,0, "%.4e");
		ImGui::InputDouble("Young's Modulus", &new_material_youngs_modulus, 0, 0, "%.3f");
		ImGui::InputDouble("Cross-Section Area", &new_material_cs_area, 0, 0, "%.3f");

		if (ImGui::Button("Create Material"))
		{
			// TODO: Add the new material to the material list
			  // Add the new material to the material list
			material_data new_material;
			new_material.material_id = material_list.size();
			new_material.material_name = new_material_name;
			new_material.mat_density = new_material_density;
			new_material.youngs_mod = new_material_youngs_modulus;
			new_material.cs_area = new_material_cs_area;
			material_list.push_back(new_material);

			 // Update the combo box
			selected_material_option = material_list.size() - 1;
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
