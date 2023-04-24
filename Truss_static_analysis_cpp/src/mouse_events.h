#pragma once
#include <glm/vec2.hpp>

class mouse_events
{
public:
	mouse_events();
	~mouse_events();
	void mouse_location(double* x, double* y);
	void pan_operation_start(double* x, double* y);
	void pan_operation_ends();
	void zoom_operation(double* e_x, double* x, double* y);
	void zoom_to_fit();
	void rotation_operation_start(double* x, double* y);
	void rotation_operation_ends();
	void left_mouse_click(double* x, double* y);
	void left_mouse_doubleclick(double* x, double* y);
	void right_mouse_click(double* x, double* y);
	void right_mouse_doubleclick(double* x, double* y);
private:
	glm::vec2 click_pt;
	glm::vec2 curr_pt;
	bool is_pan;
	bool is_rotate;
	float zoom_val;
	void pan_operation(glm::vec2* delta_d);
	void rotate_operation(glm::vec2* delta_d);
};
