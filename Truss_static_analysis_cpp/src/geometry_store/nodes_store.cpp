#include "nodes_store.h"

nodes_store::nodes_store()
{
	// Empty constructor
	node_id = -1;
	node_pt = glm::vec3(0);
	default_color = glm::vec3(0); // Store the default color of this node
	contour_color = glm::vec3(0);
}

nodes_store::nodes_store(int id, const glm::vec3& pt)
{
	// constructor
	this->node_id = id;
	this->node_pt = pt;

	default_color = glm::vec3(1.0f,1.0f,1.0f); // Store the default color of this node
	contour_color = glm::vec3(0);
}

nodes_store::~nodes_store()
{

}