#pragma once
#include <iostream>
#include <filesystem>
#include "../ImGui/stb_image.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../ImGui/stb_implement.h"

struct my_image_data
{
	int image_width = 0;
	int image_height = 0;
	unsigned int image_texture = 0;
	bool is_loaded = false;
};



class loadconstraint_window
{
public:
	loadconstraint_window();
	~loadconstraint_window();
	void bind_images();
	void render_window();
	bool is_show_window = false;
	bool is_add_constraint = false;
	bool is_add_load = false;
	int selectedOptionIndex = 0;
	float constraint_angleDegrees = 0.0f;
	float load_angleDegrees = 0.0f;
	float square_size = 50.0f;
	int loadValue = 0;
private:
	my_image_data pin_image;
	my_image_data roller_image;
	my_image_data load_image;
	void render_load_tab();
	void render_constraint_tab();
	void draw_pin_support();
	void draw_roller_support();
	void draw_load_image();
	void LoadTextureFromFile(const char* filename, unsigned int* out_texture, bool& s_IsInitialized);
	bool get_image_min_max_coord(ImVec2& window_pos, ImVec2& window_size, ImVec2& img_pos_top_left, ImVec2& img_pos_top_right,ImVec2& img_pos_bot_right, ImVec2& img_pos_bot_left,float& orientation);
};