#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <windows.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "ImGui/stb_image.h"

// Callback function for handling window resize
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void GLFWwindow_set_icon(GLFWwindow* window)
{
	// Set the window icon
	int icon_width, icon_height, numChannels;
	unsigned char* icon_image = stbi_load("./Images/innx_icon.png", &icon_width, &icon_height, &numChannels, 0);

	// Set the window icon using GLFW's API for Windows
	GLFWimage icon;
	icon.width = icon_width;
	icon.height = icon_height;
	icon.pixels = icon_image;
	glfwSetWindowIcon(window, 1, &icon);

	// Clean up the loaded image data
	stbi_image_free(icon_image);
}

int main()
{
	// Close the console window
	ShowWindow(GetConsoleWindow(), SW_HIDE); //SW_RESTORE to bring back

	// Initialize GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	// Set OpenGL version to 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Create a window
	GLFWwindow* window = glfwCreateWindow(800, 600, "Plane Truss Analysis - OpenGL 3.3", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}


	// Make the window's context current
	glfwMakeContextCurrent(window);


	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Set viewport size and register framebuffer resize callback
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	GLFWwindow_set_icon(window);

	// Setup ImGui context
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Main rendering loop
	while (!glfwWindowShouldClose(window))
	{
		// Start ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Create a menu bar
		if (ImGui::BeginMainMenuBar(), ImGuiWindowFlags_MenuBar)
		{
			// File menu item
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Import varai2D"))
				{
					// Handle menu Import varai2D
				}
				if (ImGui::MenuItem("Import raw data"))
				{
					// Handle menu Import raw data
				}
				if (ImGui::MenuItem("Export raw data"))
				{
					// Handle menu Export raw data
				}
				ImGui::EndMenu();
			}
			// Add more menu items here as needed
			ImGui::EndMainMenuBar();
		}

		// Render OpenGL graphics here
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Set the clear color to black
		glClear(GL_COLOR_BUFFER_BIT);  // Clear the color buffer



		// Render ImGui UI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap buffers
		glfwSwapBuffers(window);

		// Poll for events
		glfwPollEvents();
	}
	// Cleanup ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Terminate GLFW
	glfwTerminate();

	return 0;
}

