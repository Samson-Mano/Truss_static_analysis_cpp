#pragma once
#include <unordered_map>
#include "nodes_store.h"
#include "lines_store.h"
#include <glm/glm.hpp>

class geom_store
{

public:
	bool is_geometry_set;
	// Create an unordered_map to store nodes with ID as key
	std::unordered_map<int, nodes_store> nodeMap;
	// Create an unordered_map to store lines with ID as key
	std::unordered_map<int, lines_store> lineMap;
	geom_store();
	geom_store(const std::unordered_map<int, nodes_store>& nodeMap, std::unordered_map<int, lines_store>& lineMap);
	~geom_store();
	void deleteResources();
};
