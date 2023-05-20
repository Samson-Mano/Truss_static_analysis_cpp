#pragma once
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include "buffers/font_atlas.h"

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

	// Screen transformations
	glm::mat4 panTranslation; // Pan translataion
	float zoom_scale; // Zoom scale

	font_atlas main_font;

	geom_parameters();
	~geom_parameters();
	void init();
private:

};
