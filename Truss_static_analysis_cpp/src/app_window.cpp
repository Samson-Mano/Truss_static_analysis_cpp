#include "app_window.h"


app_window::app_window()
{
	// Main constructor
	// Initialize GLFW
	is_glwindow_success = false;
	if (!glfwInit()) {
		// ShowWindow(GetConsoleWindow(), SW_RESTORE);
		log = "Failed to initialize GLFW";
		// std::cerr << "Failed to initialize GLFW" << std::endl;
		return;
	}

	// Set OpenGL version to 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Create a window
	window = glfwCreateWindow(800, 600, "Plane Truss Finite Element Solver", nullptr, nullptr);

	if (!window) {
		log = "Failed to create GLFW window";
		// std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}

	// Make the window's context current
	glfwMakeContextCurrent(window);


	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		log = "Failed to initialize GLEW";
		// std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();
		return;
	}

	// Set viewport size and register framebuffer resize callback
	glfwGetFramebufferSize(window, &window_width, &window_height);
	glViewport(0, 0, window_width, window_height);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	// Set the icon for the window
	GLFWwindow_set_icon(window);

	// Setup ImGui context
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Window initialize success
	is_glwindow_success = true;
}


app_window::~app_window()
{
	// Destructor
	// Cleanup ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Terminate GLFW
	glfwTerminate();
}

void app_window::app_render()
{
	// Create a custom font for the menu bar
	imgui_font = ImGui::GetIO().Fonts->AddFontFromFileTTF("./Images/font/FreeSans.ttf", 18);

	// Main rendering loop
	while (!glfwWindowShouldClose(window))
	{
		// Start ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Change the font for the menu bar
		ImGui::PushFont(imgui_font);

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

		// Pop the custom font after using it
		ImGui::PopFont();

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
}

// Static callback function for framebuffer size changes
// static keyword makes the function a class-level function rather than an instance-level function
// allows it to be used as a callback function for the GLFW library
void app_window::framebufferSizeCallback(GLFWwindow* window, int window_width, int window_height)
{
	glViewport(0, 0, window_width, window_height);
}

void app_window::GLFWwindow_set_icon(GLFWwindow* window)
{
	// Get the image
	stb_implement stb("./Images/innx_icon.png");

	// Set the window icon using GLFW's API for Windows
	GLFWimage icon;
	icon.width = stb.image_width;
	icon.height = stb.image_height;
	icon.pixels = stb.image;
	glfwSetWindowIcon(window, 1, &icon);

	// Image memory is released at the end
}
