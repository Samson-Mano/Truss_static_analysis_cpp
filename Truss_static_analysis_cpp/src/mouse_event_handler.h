#pragma once
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "mouse_events.h"
#include "../ImGui/imgui.h"
#include "geometry_store/geom_store.h"
#include "loadconstraint_window.h"
#include "material_window.h"

class mouse_event_handler {
private:
	bool isCtrlDown = false; // Flag to indicate if Ctrl key is currently pressed
	mouse_events mouse_click;
	glm::vec2 last_pt;
	double lastClickTime;
	int lastButton;
	int clickCount;
public:
	// Constructor
	mouse_event_handler();
	~mouse_event_handler();

	void add_geometry_ptr(geom_store* geom, int* window_width, int* window_height,
		loadconstraint_window* ct_window,material_window* mat_window);
	// Mouse button callback function
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	// Mouse move callback function
	static void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
	// Mouse scroll callback function
	static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	// Key down callback function
	static void keyDownCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	// Handle mouse button event
	void handleMouseButton(int button, int action, int mods, double xpos, double ypos);
	// Handle mouse move event
	void handleMouseMove(double xpos, double ypos);
	// Handle mouse scroll event
	void handleMouseScroll(double xoffset, double yoffset, double xpos, double ypos);
	// Handle key down event
	void handleKeyDown(int key, int scancode, int action, int mods);
	// Zoom to fit
	void zoom_to_fit();
};