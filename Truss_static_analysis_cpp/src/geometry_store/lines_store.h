#pragma once
#include <glm/vec3.hpp>
#include "nodes_store.h"

class lines_store
{
private:

public:
	int line_id; // ID of the line
	nodes_store s_nd; // start node
	nodes_store e_nd; // end node
	lines_store();
	lines_store(int id, const nodes_store& startNode, const nodes_store& endNode); // Constructor overload to initialize line_id, s_nd, and e_nd directly
	~lines_store();
};
