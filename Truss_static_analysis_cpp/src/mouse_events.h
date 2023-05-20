#pragma once
#include <iostream>
#include <glm/vec2.hpp>
#include "geometry_store/geom_store.h"
#include "loadconstraint_window.h"

class mouse_events
{
public:
	mouse_events();
	~mouse_events();
	void mouse_location(glm::vec2& loc);
	void pan_operation_start(glm::vec2& loc);
	void pan_operation_ends();
	void zoom_operation(double& e_x, glm::vec2& loc);
	void zoom_to_fit();
	void rotation_operation_start(glm::vec2& loc);
	void rotation_operation_ends();
	void left_mouse_click(glm::vec2& loc);
	void left_mouse_doubleclick(glm::vec2& loc);
	void right_mouse_click(glm::vec2& loc);
	void right_mouse_doubleclick(glm::vec2& loc);
	void add_geometry_ptr(geom_store* geom, int* window_width, int* window_height,
		loadconstraint_window* ct_window, material_window* mat_window);
private:
	geom_store* geom;
	int* window_width;
	int* window_height;
	loadconstraint_window* ct_window;
	material_window* mat_window;

	glm::vec2 click_pt;
	glm::vec2 curr_pt;
	glm::vec2 prev_translation;
	glm::vec2 total_translation;
	bool is_pan;
	bool is_rotate;
	float zoom_val;
	void pan_operation(glm::vec2& current_translataion);
	void rotate_operation(glm::vec2& delta_d);
	glm::vec2 intellizoom_normalized_screen_pt(glm::vec2 delta_d);
};
