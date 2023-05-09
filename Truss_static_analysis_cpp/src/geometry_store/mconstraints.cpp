#include "mconstraints.h"

mconstraints::mconstraints()
	:constraint_count(0)
{
	// Constructor
}

mconstraints::~mconstraints()
{
	// Destructor
}

void mconstraints::add_constraint(int& node_id, nodes_store* node, int& constraint_type, float& constraint_angle)
{
	// Add constraints
	constraint_data temp_c_data;
	temp_c_data.node_id = node_id;
	temp_c_data.node = node;
	temp_c_data.constraint_type = constraint_type;
	temp_c_data.constraint_angle = constraint_angle;

	// Insert the constarint data to unordered map
	// Searching for node_id
	if (c_data.find(node_id) != c_data.end())
	{
		// Node is already have constraint
		// so remove the constraint
		c_data[node_id] = temp_c_data;
	}
	else
	{
		// Insert the constraint to nodes
		c_data.insert({ node_id, temp_c_data });
	}


	constraint_count++;
}

void mconstraints::delete_constraint(int& node_id)
{
	if (constraint_count != 0)
	{
		// Remove the constarint data to unordered map
		// Searching for node_id
		// Check there is already a constraint in the found node
		if (c_data.find(node_id) != c_data.end())
		{
			// Node is already have constraint
			// so remove the constraint
			c_data.erase(node_id);

			// adjust the constraint count
			constraint_count--;
		}
	}
}