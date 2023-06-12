#include "app_window.h"
#include "geometry_store/buffers/IndexBuffer.h"
#include "geometry_store/buffers/VertexArray.h"
#include "geometry_store/buffers/VertexBuffer.h"
#include "geometry_store/buffers/VertexBufferLayout.h"

int app_window::window_width = 800;
int app_window::window_height = 600;
bool app_window::isWindowSizeChanging = false;

app_window::app_window()
	:geom(), ct_window()
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
	window = glfwCreateWindow(window_width, window_height, "Plane Truss Finite Element Solver", nullptr, nullptr);

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

	//// Maximize the window
	//glfwMaximizeWindow(window);

	// Set viewport size and register framebuffer resize callback
	glfwGetFramebufferSize(window, &window_width, &window_height);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	geom.updateWindowDimension(window_width, window_height);

	// Set the icon for the window
	GLFWwindow_set_icon(window);

	// Window initialize success
	is_glwindow_success = true;

	// Set the mouse button callback function with the user pointer pointing to the mouseHandler object
	glfwSetWindowUserPointer(window, &mouse_Handler);

	// Passing the address of geom and window dimensions to mouse handler
	mouse_Handler.add_geometry_ptr(&geom, &window_width, &window_height, &ct_window,&mat_window);

	// Pass the address of options window, material window, solver window
	geom.add_window_ptr(&op_window,&mat_window,&fe_window);

	glfwSetMouseButtonCallback(window, mouse_event_handler::mouseButtonCallback);

	// Set the mouse move callback function with the user pointer pointing to the mouseHandler object
	glfwSetCursorPosCallback(window, mouse_event_handler::mouseMoveCallback);

	// Set the mouse scroll callback function with the user pointer pointing to the mouseHandler object
	glfwSetScrollCallback(window, mouse_event_handler::mouseScrollCallback);

	// Set key input callback function with the user pointer pointing to the mouseHandler object
	glfwSetKeyCallback(window, mouse_event_handler::keyDownCallback);


	// Setup ImGui context
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	ImGui::StyleColorsDark();  // Set the default ImGui dark theme colors

	// Modify specific colors
	ImVec4 background_color = ImVec4(0.32f, 0.32f, 0.32f, 0.8f);  // Background color
	ImVec4 text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // Text color
	ImVec4 button_color = ImVec4(0.0f, 0.5f, 5.0f, 1.0f);  // Button color

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = background_color;  // Set the background color
	style.Colors[ImGuiCol_Text] = text_color;  // Set the text color
	style.Colors[ImGuiCol_Button] = button_color;  // Set the button color

	// Apply the modified style
	// ImGui::StyleColorsDark();

	framebufferSizeCallback(window, window_width, window_height);
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
	ImGuiIO& io = ImGui::GetIO();
	imgui_font = io.Fonts->AddFontFromFileTTF("./Images/font/FreeSans.ttf", 18);

	geom.init();
	ct_window.bind_images();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(3.0f);

	// Main rendering loop
	while (!glfwWindowShouldClose(window))
	{
		// Start ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// menu events
		menu_events();

		// Render OpenGL graphics here
		glClearColor(geom.geom_param.geom_colors.background_color.x,
			geom.geom_param.geom_colors.background_color.y,
			geom.geom_param.geom_colors.background_color.z, 1.0f);  // Set the clear color to black
		glClear(GL_COLOR_BUFFER_BIT);  // Clear the color buffer

		// Window size change event
		if (isWindowSizeChanging == true)
		{
			geom.updateWindowDimension(window_width, window_height);
			mouse_Handler.zoom_to_fit();
		}
		isWindowSizeChanging = false;

		// Paint the geometry
		geom.paint_geometry();


		// Render ImGui UI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap buffers
		glfwSwapBuffers(window);

		// Poll for events
		glfwPollEvents();
	}
}

void app_window::menu_events()
{
	// Control the menu events
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
				menu_click.update_event(import_varai2d, geom);
				isWindowSizeChanging = true;
			}
			if (ImGui::MenuItem("Import *.dxf file"))
			{
				// Handle menu Import AutoCAD dxf file
				menu_click.update_event(import_dxf_data, geom);
				isWindowSizeChanging = true;
			}
			if (ImGui::MenuItem("Import raw data"))
			{
				// Handle menu Import raw data
				menu_click.update_event(import_raw_data, geom);
				isWindowSizeChanging = true;
			}
			if (ImGui::MenuItem("Export raw data"))
			{
				// Handle menu Export raw data
				menu_click.update_event(export_raw_data, geom);
			}
			if (ImGui::MenuItem("Exit"))
			{
				// Handle menu Exit
				exit(0);
				menu_click.update_event(exit_p, geom);
			}
			ImGui::EndMenu();
		}
		// Edit menu item
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Constraints/ Loads"))
			{
				// Handle menu Add Load & Add Constraint
				if (geom.is_geometry_set == true)
				{
					ct_window.is_show_window = true;
				}
			}
			if (ImGui::MenuItem("Material"))
			{
				// Handle menu Edit Materials
				if (geom.is_geometry_set == true)
				{
					mat_window.is_show_window = true;
				}
			}
			if (ImGui::MenuItem("View Options"))
			{
				// Handle menu View Options of the app
				op_window.is_show_window = true;
			}
			ImGui::EndMenu();
		}
		// Solve menu item
		if (ImGui::BeginMenu("Solve"))
		{
			if (ImGui::MenuItem("FE Analysis"))
			{
				// Handle menu FE Analysis
				if (geom.is_geometry_set == true)
				{
					fe_window.execute_open = true;
					fe_window.is_show_window = true;
				}
			}
			ImGui::EndMenu();
		}
		// Add more menu items here as needed
		ImGui::EndMainMenuBar();
	}

	// Execute window render operation
	ct_window.render_window();
	op_window.render_window();
	mat_window.render_window();
	fe_window.render_window();

// Pop the custom font after using it
	ImGui::PopFont();
}

// Static callback function for framebuffer size changes
// static keyword makes the function a class-level function rather than an instance-level function
// allows it to be used as a callback function for the GLFW library
void app_window::framebufferSizeCallback(GLFWwindow* window, int window_width, int window_height)
{
	// Triggers when the openGL window is resized
	app_window::window_width = window_width;
	app_window::window_height = window_height;

	int max_dim = window_width > window_height ? window_width : window_height;
	int x_offset = (max_dim - window_width) / 2; // Calculate x offset to center the viewport
	int y_offset = (max_dim - window_height) / 2; // Calculate y offset to center the viewport

	// Set the viewport to the maximum dimension and center it at (0, 0)
	glViewport(-x_offset, -y_offset, max_dim, max_dim);

	isWindowSizeChanging = true;
	// glViewport(0, 0, max_size, max_size);
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
