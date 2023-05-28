#include "geom_store.h"

geom_parameters::geom_parameters()
	:window_width(0), window_height(0), min_b(0), max_b(0), geom_bound(0),
	center(0), modelMatrix(0), geom_scale(1.0f), panTranslation(1.0f), zoom_scale(1.0f), geom_transparency(1.0f), defl_scale(0.01f)
{
	// Constructutor
}

geom_parameters::~geom_parameters()
{
}

void geom_parameters::init()
{
	// Initialize the paramters
	main_font.create_atlas();

	// Initialize the color theme
	geom_colors.background_color = glm::vec3(0.62f, 0.62f, 0.62f);
	geom_colors.node_color = glm::vec3(0.0f, 0.0f, 0.4f);
	geom_colors.line_color = glm::vec3(0.0f, 0.2f, 0.6f);
	geom_colors.constraint_color = glm::vec3(0.6f, 0.0f, 0.6f);
	geom_colors.load_color = glm::vec3(0.0f, 1.0f, 0.0f);

}