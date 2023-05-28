#pragma once
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include "buffers/font_atlas.h"

struct geom_color_theme
{
	glm::vec3 background_color;
	glm::vec3 node_color;
	glm::vec3 line_color;
	glm::vec3 constraint_color;
	glm::vec3 load_color;
};

class geom_parameters
{
public:
	// Standard sizes
	const float font_size = 16.0f * std::pow(10, -5);
	const float node_circle_radii = 0.005f;

	// Window size
	int window_width;
	int window_height;

	glm::vec3 min_b; // (min_x, min_y,0)
	glm::vec3 max_b; // (max_x, max_y,0)
	glm::vec3 geom_bound; // Bound magnitude
	glm::vec3 center; // center of the geometry
	glm::mat4 modelMatrix; // Geometry model matrix
	float geom_scale; // Scale of the geometry
	float geom_transparency; // Value to control the geometry transparency
	float defl_scale; // Value of deflection scale

	// Screen transformations
	glm::mat4 panTranslation; // Pan translataion
	float zoom_scale; // Zoom scale

	// Standard colors
	geom_color_theme geom_colors;

	font_atlas main_font;

	geom_parameters();
	~geom_parameters();
	void init();
private:

};
