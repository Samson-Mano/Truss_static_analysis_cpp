#pragma once
#include <unordered_map>
#include "nodes_store.h"

struct load_data
{
	int node_id;
	nodes_store* node;
	float load_value;
	float load_angle;
};

class mloads
{
public:
	// Create an unordered_map to store constraints with node ID as key
	int load_count;
	std::unordered_map<int, load_data> l_data;
	mloads();
	~mloads();
	void add_load(int& node_id, nodes_store* node, float& load_value, float& load_angle);
	void delete_load(int& node_id);
	void delete_all();
private:

};