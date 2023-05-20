#include "mouse_event_handler.h"

mouse_event_handler::mouse_event_handler()
	:isCtrlDown(false),
	mouse_click(), last_pt(0), lastClickTime(0.0), lastButton(-1), clickCount(0)
{
	// Constructor
}

mouse_event_handler::~mouse_event_handler()
{
	// Destructor
}

void mouse_event_handler::add_geometry_ptr(geom_store* geom, int* window_width, int* window_height,
	loadconstraint_window* ct_window,material_window* mat_window)
{
	mouse_click.add_geometry_ptr(geom, window_width, window_height, ct_window, mat_window);
}

// Mouse button callback function
void mouse_event_handler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse) {
		return;
	}

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);  // Get mouse position
	mouse_event_handler* handler = static_cast<mouse_event_handler*>(glfwGetWindowUserPointer(window));
	handler->handleMouseButton(button, action, mods, xpos, ypos);
}

// Mouse move callback function
void mouse_event_handler::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
	mouse_event_handler* handler = static_cast<mouse_event_handler*>(glfwGetWindowUserPointer(window));
	handler->handleMouseMove(xpos, ypos);
}

// Mouse scroll callback function
void mouse_event_handler::mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Retrieve the current mouse cursor position using GLFW
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	mouse_event_handler* handler = static_cast<mouse_event_handler*>(glfwGetWindowUserPointer(window));
	handler->handleMouseScroll(xoffset, yoffset, xpos, ypos);
}

// Key down callback function
void mouse_event_handler::keyDownCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	mouse_event_handler* handler = static_cast<mouse_event_handler*>(glfwGetWindowUserPointer(window));
	handler->handleKeyDown(key, scancode, action, mods);
}


void mouse_event_handler::handleMouseButton(int button, int action, int mods, double xpos, double ypos)
{
	// Get current time
	double currentTime = glfwGetTime();
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			if ((currentTime - lastClickTime) < 0.5)
			{
				clickCount++;
			}
			else
			{
				clickCount = 1;
			}

			// Left Mouse down
			last_pt = glm::vec2(xpos,ypos);
			lastClickTime = currentTime;
			lastButton = GLFW_MOUSE_BUTTON_LEFT;

			if (isCtrlDown == true)
			{
				glm::vec2 loc = glm::vec2(xpos, ypos);
				mouse_click.rotation_operation_start(loc);
			}
		}
		else if (action == GLFW_RELEASE)
		{
			// Left Mouse up
			mouse_click.rotation_operation_ends();

			// Calculate mouse move distance
			double deltaX = xpos - last_pt.x;
			double deltaY = ypos - last_pt.y;
			
			// Update last position
			last_pt = glm::vec2(xpos, ypos);

			// Check if it's a click or drag
			if (deltaX == 0.0 && deltaY == 0.0 && (currentTime - lastClickTime) < 0.5 && lastButton == GLFW_MOUSE_BUTTON_LEFT)
			{
				// Left Mouse click
					glm::vec2 loc = glm::vec2(xpos, ypos);
				if (clickCount == 2)
				{
					// Double click
					mouse_click.left_mouse_doubleclick(loc);
				}
				else if (clickCount == 1)
				{
					// Single click
					mouse_click.left_mouse_click(loc);
				}
			}
		}
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
		{
			if ((currentTime - lastClickTime) < 0.5)
			{
				clickCount++;
			}
			else
			{
				clickCount = 1;
			}

			// Right Mouse down
			last_pt = glm::vec2(xpos, ypos);
			lastClickTime = currentTime;
			lastButton = GLFW_MOUSE_BUTTON_RIGHT;

			if (isCtrlDown == true)
			{
				// Pan operation start
				glm::vec2 loc = glm::vec2(xpos, ypos);
				mouse_click.pan_operation_start(loc);
			}
		}
		else if (action == GLFW_RELEASE)
		{
			// Right Mouse up
			mouse_click.pan_operation_ends();

			// Calculate mouse move distance
			double deltaX = xpos - last_pt.x;
			double deltaY = ypos - last_pt.y;
			// Update last position
			last_pt = glm::vec2(xpos, ypos);

			// Check if it's a click or drag
			if (deltaX == 0.0 && deltaY == 0.0 && (currentTime - lastClickTime) < 0.5 && lastButton == GLFW_MOUSE_BUTTON_RIGHT)
			{
				// Right Mouse click
				glm::vec2 loc = glm::vec2(xpos, ypos);
				if (clickCount == 2)
				{
					// Double click
					mouse_click.right_mouse_doubleclick(loc);
				}
				else if (clickCount == 1)
				{
					// Single click
					mouse_click.right_mouse_click(loc);
				}
			}
		}
	}


	// Access member variables or call member methods here
	// std::cout << "Mouse button: " << button << ", action: " << action << ", mods: " << mods << std::endl;
}


void mouse_event_handler::handleMouseMove(double xpos, double ypos)
{
	// Mouse move operation
	if (isCtrlDown == true)
	{
		glm::vec2 loc = glm::vec2(xpos, ypos);
		mouse_click.mouse_location(loc);
	}
}

void mouse_event_handler::handleMouseScroll(double xoffset, double yoffset, double xpos, double ypos)
{
	// Mouse scroll operation
	if (isCtrlDown == true)
	{
		glm::vec2 loc = glm::vec2(xpos, ypos);
		mouse_click.zoom_operation(yoffset, loc);
	}
}

void mouse_event_handler::handleKeyDown(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
	{
		if (action == GLFW_PRESS)
		{
			isCtrlDown = true;
		}
		else if (action == GLFW_RELEASE)
		{
			isCtrlDown = false;
		}
	}

	if (isCtrlDown && key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		// Ctrl + F combination detected 
		// Perform zoom to fit
		mouse_click.zoom_to_fit();
	}
}

void  mouse_event_handler::zoom_to_fit()
{
	// Used during window resize
	// Perform zoom to fit 
	mouse_click.zoom_to_fit();
}