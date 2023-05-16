#pragma once
#include <iostream>
#include <filesystem>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../ImGui/stb_implement.h"


struct material_data
{
	std::string material_name;
	double youngs_mod;
	double mat_density;
	double cs_area;
};

class material_window
{
public:
	bool is_show_window = false;
	int selected_material_option = 0;
	std::vector<material_data> material_list;

	material_window();
	~material_window();
	void render_window();
	void render_assignmaterial_tab();
	void render_creatematerial_tab();
private:


};