#include "nodes_store.h"

nodes_store::nodes_store()
{
	// Empty constructor
}

nodes_store::nodes_store(int id, const glm::vec3& pt)
{
	// constructor
	this->node_id = id;
	this->node_pt = pt;
}

nodes_store::~nodes_store()
{

}