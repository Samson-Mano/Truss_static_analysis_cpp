#include "material_window.h"

material_window::material_window()
	: is_show_window(false)
{
	// Constructor
	// ton	mm	s	N	MPa	N-mm
	// 7.83e-09	Density 2.07e+05 Youngs Modulus	9.806e+03 N/mm2 cross section area (209 x 9.6 mm pipe) = 6014
	material_data default_material;
	default_material.material_name = "Default material";
	default_material.mat_density = 7.83 * std::pow(10, -9); // tons/mm3
	default_material.youngs_mod = 2.07 * std::pow(10, 5); // MPa
	default_material.cs_area = 6014; // mm2

	material_list.push_back(default_material);
}

material_window::~material_window()
{
	// Destructor
}

void material_window::render_window()
{
	if (is_show_window == false)
		return;

	ImGui::Begin("Materials");

	// Convert material_list to a vector of const char*
	std::vector<const char*> material_names;
	for (const auto& mat : material_list) {
		material_names.push_back(mat.material_name.c_str());
	}

	ImGui::Combo("Select Material", &selected_material_option, material_names.data(), material_names.size());

	// Get selected material
	const material_data& selected_material_data = material_list[selected_material_option];

	ImGui::Text("Selected Material: %s", selected_material_data.material_name.c_str());
	ImGui::Text("Young's Modulus: %.3f", selected_material_data.youngs_mod);
	ImGui::Text("Density: %.3f", selected_material_data.mat_density);
	ImGui::Text("Cross-Section Area: %.3f", selected_material_data.cs_area);

	// Diable delete if the selected option is Default (0)
	const bool is_delete_button_disabled = selected_material_option ==0?true:false;
	ImGui::BeginDisabled(is_delete_button_disabled);
	if (ImGui::Button("Delete Material")) {
		// Delete material
	}
	ImGui::EndDisabled();

	// Add some spacing before the "Create Material" header
	ImGui::Spacing();
	ImGui::Spacing();

	// Create material dropdown
	if (ImGui::CollapsingHeader("Create Material "))
	{
		static char new_material_name[256] = "New Material";
		static double new_material_density = 7.865 * std::pow(10, -1);
		static double new_material_youngs_modulus = 210000.0;
		static double new_material_cs_area = 8000.0;

		ImGui::InputText("Material Name", new_material_name, IM_ARRAYSIZE(new_material_name));
		ImGui::InputDouble("Density", &new_material_density);
		ImGui::InputDouble("Young's Modulus", &new_material_youngs_modulus);
		ImGui::InputDouble("Cross-Section Area", &new_material_cs_area);

		if (ImGui::Button("Create Material"))
		{
			// TODO: Add the new material to the material list
			  // Add the new material to the material list
			material_data new_material;
			new_material.material_name = new_material_name;
			new_material.mat_density = new_material_density;
			new_material.youngs_mod = new_material_youngs_modulus;
			new_material.cs_area = new_material_cs_area;
			material_list.push_back(new_material);

			// Update the combo box
			//const int new_material_index = material_list.size() - 1;
			//selected_material_option = new_material_index;
		}
	}

	ImGui::End();
}

void material_window::render_assignmaterial_tab()
{
	// Assign Material


}

void material_window::render_creatematerial_tab()
{
	// Create Material

}
