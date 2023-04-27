#pragma once
#include <glm/vec3.hpp>

class nodes_store
{
private:

public:
	int node_id;
	glm::vec3 node_pt;
	nodes_store();
	nodes_store(int id, const glm::vec3& pt); // Constructor to initialize node_id and node_pt directly
	~nodes_store();
};
