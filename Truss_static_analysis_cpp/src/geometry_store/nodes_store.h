#pragma once
#include <glm/vec3.hpp>

class nodes_store
{
private:

public:
	int node_id;
	glm::vec3 node_pt;
	glm::vec3 default_color; // Store the default color of this node
	glm::vec3 contour_color;
	nodes_store();
	~nodes_store();
	void add_node(int& node_id, const glm::vec3& node_pt); // Constructor to initialize node_id and node_pt directly
};
