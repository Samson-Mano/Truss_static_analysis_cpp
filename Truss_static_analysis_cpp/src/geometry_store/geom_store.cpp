#include "geom_store.h"

geom_store::geom_store()
{
	// Empty constructor
	is_geometry_set = false;
}

geom_store::geom_store(const std::unordered_map<int, nodes_store>& nodeMap, std::unordered_map<int, lines_store>& lineMap)
{
	// Constructor
	this->nodeMap = nodeMap;
	this->lineMap = lineMap;
	is_geometry_set = true;
}

geom_store::~geom_store()
{
	// Destructor
	deleteResources();
}

void geom_store::deleteResources()
{
	is_geometry_set = false;
	// Call the destructor for each nodes_store object in the nodeMap
	for (auto& node : nodeMap) {
		node.second.~nodes_store();
	}
	// Call the destructor for each lines_store object in the lineMap
	for (auto& line : lineMap) {
		line.second.~lines_store();
	}
	// Clear the nodeMap and lineMap
	nodeMap.clear();
	lineMap.clear();
}