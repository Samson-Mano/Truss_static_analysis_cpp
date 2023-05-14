#pragma once
#include <unordered_map>
#include "nodes_store.h"

struct constraint_data
{
	int node_id;
	nodes_store* node;
	int constraint_type;
	float constraint_angle;
};

class mconstraints
{
public:
	// Create an unordered_map to store constraints with node ID as key
	int constraint_count;
	std::unordered_map<int, constraint_data> c_data;
	mconstraints();
	~mconstraints();
	void add_constraint(int& node_id, nodes_store* node, int& constraint_type,float& constraint_angle);
	void delete_constraint(int& node_id);
	void delete_all();
private:

};
