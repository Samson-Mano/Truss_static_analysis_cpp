#include "geom_store.h"

geom_parameters::geom_parameters()
	:window_width(0), window_height(0), min_b(0),max_b(0),geom_bound(0),
	center(0),modelMatrix(0),geom_scale(1.0f),panTranslation(1.0f),zoom_scale(1.0f)
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
}