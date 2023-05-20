#pragma once
#include <unordered_map>
#include "nodes_store_list.h"

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
	void init(geom_parameters* geom_param_ptr);
	void add_constraint(int& node_id, nodes_store* node, int& constraint_type,float& constraint_angle);
	void delete_constraint(int& node_id);
	void delete_all();
	void set_buffer();
	void update_buffer();
	void paint_constraints();
	void update_geometry_matrices(bool is_modelmatrix, bool is_pantranslation, bool is_zoomtranslation);
private:
	geom_parameters* geom_param_ptr;
	gBuffers constraint_buffer;
	Texture constraint_texture_pin;
	Texture constraint_texture_roller;
	shader constraint_shader;

	void set_constraint_vertices(float* constraint_vertices, unsigned int& constraint_v_index, 
		nodes_store* node, float constraint_angle, unsigned int constraint_type);
	void set_constraint_indices(unsigned int* constraint_vertex_indices, unsigned int& constraint_i_index);
};
