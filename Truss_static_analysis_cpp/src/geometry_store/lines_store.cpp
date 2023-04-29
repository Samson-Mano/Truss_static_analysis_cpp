#include "lines_store.h"


lines_store::lines_store()
{
	// Empty constructor
	line_id = -1;
}

lines_store::lines_store(int id, const nodes_store& startNode, const nodes_store& endNode)
{
	this->line_id = id;
	this->s_nd = startNode;
	this->e_nd = endNode;
}

lines_store::~lines_store()
{
	// Destructor
}
