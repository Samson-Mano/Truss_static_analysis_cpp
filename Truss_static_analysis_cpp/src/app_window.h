#pragma once
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "stb_implement.h"

class app_window
{
private:
	GLFWwindow* window;
	int window_width;
	int window_height;
	ImFont* imgui_font;
public:
	bool is_glwindow_success;
	const char* log;

	// Constructor and Destructor
	app_window();

	~app_window();

	// Functions
	void app_render();

	static void framebufferSizeCallback(GLFWwindow* window, int window_width, int window_height);

	void GLFWwindow_set_icon(GLFWwindow* window);
};