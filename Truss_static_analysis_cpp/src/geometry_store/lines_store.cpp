#include "lines_store.h"


lines_store::lines_store()
	: line_id(-1), startNode(), endNode()
{
	// Empty constructor
}

lines_store::~lines_store()
{
	// Destructor
}

void lines_store::add_line(int& line_id, const nodes_store& startNode, const nodes_store& endNode)
{
	// Add the line
	this->line_id = line_id;
	this->startNode = startNode;
	this->endNode = endNode;
}
