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
	const float font_size = static_cast<float>(16.0f * std::pow(10, -5));
	const float node_circle_radii = 0.005f;

	// Precision for various values
	const int length_precision = 3;
	const int coord_precision = 3;
	const int load_precision = 2;
	const int defl_precision = 6;
	const int mforce_precision = 2;
	const int mstress_precision = 3;

	// File path
	std::filesystem::path resourcePath;

	// Window size
	int window_width = 800;
	int window_height = 600;

	glm::vec3 min_b = glm::vec3(0); // (min_x, min_y,0)
	glm::vec3 max_b = glm::vec3(0); // (max_x, max_y,0)
	glm::vec3 geom_bound = glm::vec3(0); // Bound magnitude
	glm::vec3 center = glm::vec3(0); // center of the geometry
	glm::mat4 modelMatrix = glm::mat4(0); // Geometry model matrix
	float geom_scale = 1.0f; // Scale of the geometry
	float geom_transparency = 1.0f; // Value to control the geometry transparency
	float defl_scale = 0.01f; // Value of deflection scale

	// Screen transformations
	glm::mat4 panTranslation =  glm::mat4(1); // Pan translataion
	float zoom_scale = 1.0f; // Zoom scale

	// Standard colors
	geom_color_theme geom_colors;

	font_atlas main_font;

	geom_parameters();
	~geom_parameters();
	void init();
private:

};
