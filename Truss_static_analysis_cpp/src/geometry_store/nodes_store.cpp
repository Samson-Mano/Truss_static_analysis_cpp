#include "nodes_store.h"

nodes_store::nodes_store()
	:node_id(-1), node_pt(0), default_color(0), contour_color(0)
{
	// Empty constructor
}

nodes_store::~nodes_store()
{

}

void nodes_store::add_node(int& node_id, const glm::vec3& node_pt)
{
	// Add Node
	this->node_id = node_id;
	this->node_pt = node_pt;

	this->default_color = glm::vec3(1.0f,1.0f,1.0f); // Store the default color of this node
	this->contour_color = glm::vec3(0);
}
