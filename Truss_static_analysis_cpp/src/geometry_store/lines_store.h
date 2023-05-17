#pragma once
#include <glm/vec3.hpp>
#include "nodes_store.h"

class lines_store
{
private:

public:
	int line_id; // ID of the line
	int material_id; // Material ID of the line member
	nodes_store startNode; // start node
	nodes_store endNode; // end node
	lines_store();
	~lines_store();
	void add_line(int& id, const nodes_store& startNode, const nodes_store& endNode,int material_id); // Constructor overload to initialize line_id, s_nd, and e_nd directly
};
