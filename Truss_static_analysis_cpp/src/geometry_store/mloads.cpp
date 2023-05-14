#include "mloads.h"

mloads::mloads()
	:load_count(0)
{
	// Constructor
}

mloads::~mloads()
{
	// Destructor
}

void mloads::add_load(int& node_id, nodes_store* node, float& load_value, float& load_angle)
{
	if (load_value == 0)
		return;

	// Add constraints
	load_data temp_l_data;
	temp_l_data.node_id = node_id;
	temp_l_data.node = node;
	temp_l_data.load_value = load_value;
	temp_l_data.load_angle = load_angle;

	// Insert the constarint data to unordered map
	// Searching for node_id
	if (l_data.find(node_id) != l_data.end())
	{
		// Node is already have constraint
		// so remove the constraint
		l_data[node_id] = temp_l_data;
	}
	else
	{
		// Insert the constraint to nodes
		l_data.insert({ node_id, temp_l_data });
	}


	load_count++;
}

void mloads::delete_load(int& node_id)
{
	if (load_count != 0)
	{
		// Remove the load data to unordered map
		// Searching for node_id
		// Check there is already a constraint in the found node
		if (l_data.find(node_id) != l_data.end())
		{
			// Node is already have load
			// so remove the load
			l_data.erase(node_id);

			// adjust the constraint count
			load_count--;
		}
	}
}

void mloads::delete_all()
{
	// Delete all the resources
	load_count = 0;
	l_data.clear();
}
