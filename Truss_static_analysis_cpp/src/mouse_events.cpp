#include "mouse_events.h"

mouse_events::mouse_events()
	:click_pt(0), curr_pt(0), total_translation(0), prev_translation(0), 
	is_pan(false), is_rotate(false), zoom_val(1.0f)
{
	// Constructor
}

mouse_events::~mouse_events()
{
	// Destructor
}

void mouse_events::add_geometry_ptr(geom_store* geom)
{
	this->geom = geom;
}

void mouse_events::mouse_location(double& x, double& y)
{
	// Copy the current mouse location only when for pan or zoom operation
	if (is_pan == true)
	{
		// Pan operation in progress
		curr_pt = glm::vec2(x, -1.0f*y);
		glm::vec2 delta_d = click_pt - curr_pt;
		// pan
		// std::cout << "Pan translation "<<delta_d.x<<", " << delta_d.y << std::endl;
		pan_operation(delta_d);
	}

	if (is_rotate == true)
	{
		// Rotate operation in progress
		curr_pt = glm::vec2(x, y);
		glm::vec2 delta_d = click_pt - curr_pt;
		// rotate
		rotate_operation(delta_d);
	}
}

void mouse_events::pan_operation_start(double& x, double& y)
{
	// Pan operation start
	is_pan = true;
	// Note the click point when the pan operation start
	click_pt = glm::vec2(x, -1.0f * y);
	std::cout << "Pan Operation Start" << std::endl;
}

void mouse_events::pan_operation(glm::vec2& current_translataion)
{
	// Pan operation in progress
	total_translation = prev_translation + current_translataion;

	geom->pan_geometry(total_translation);
}

void mouse_events::pan_operation_ends()
{
	// Pan operation complete
	prev_translation = total_translation;
	is_pan = false;
	std::cout << "Pan Operation End" << std::endl;
}

void mouse_events::rotation_operation_start(double& x, double& y)
{
	// Rotate operation start
	is_rotate = true;
	// Note the click point when the pan operation start
	click_pt = glm::vec2(x, y);
	std::cout << "Rotate Operation Start" << std::endl;
}

void mouse_events::rotate_operation(glm::vec2& delta_d)
{
	// Rotate operation in progress

}

void mouse_events::rotation_operation_ends()
{
	// Rotate operation complete
	is_rotate = false;
	std::cout << "Rotate Operation End" << std::endl;
}

void mouse_events::zoom_operation(double& e_delta, double& x, double& y)
{
	// Zoom operation
	if ((e_delta) > 0)
	{
		// Scroll Up
		if (zoom_val < 1000)
		{
			zoom_val = zoom_val + 0.1f;
		}
	}
	else if ((e_delta) < 0)
	{
		// Scroll Down
		if (zoom_val > 0.101)
		{
			zoom_val = zoom_val - 0.1f;
		}
	}
	std::cout << "Zoom val: " << zoom_val << std::endl;
}

void  mouse_events::zoom_to_fit()
{
	// Zoom to fit the model
	prev_translation = glm::vec2(0);
	zoom_val = 1.0f;
	geom->zoomfit_geometry();
	std::cout << "Zoom val: " << zoom_val << std::endl;
}

void mouse_events::left_mouse_click(double& x, double& y)
{
	// Left mouse single click
	std::cout << "Left mouse single click" << std::endl;
}

void mouse_events::left_mouse_doubleclick(double& x, double& y)
{
	// Left mouse double click
	std::cout << "Left mouse double click" << std::endl;
}

void mouse_events::right_mouse_click(double& x, double& y)
{
	// Right mouse single click
	std::cout << "Right mouse single click" << std::endl;
}

void mouse_events::right_mouse_doubleclick(double& x, double& y)
{
	// Right mouse double click
	std::cout << "Right mouse double click" << std::endl;
}