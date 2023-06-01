#pragma once
#include <unordered_map>
#include "nodes_store_list.h"

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
	float max_load = 0.0f;

	mloads();
	~mloads();
	void init(geom_parameters* geom_param_ptr);
	void add_load(int& node_id, nodes_store* node, float& load_value, float& load_angle);
	void delete_load(int& node_id);
	void delete_all();
	void set_buffer();
	void update_buffer();
	void paint_loads();
	void paint_load_labels();
	void update_geometry_matrices(bool is_modelmatrix, bool is_pantranslation, bool is_zoomtranslation, bool set_transparency,bool is_rslt);
private:
	geom_parameters* geom_param_ptr;
	gBuffers loadarrowhead_buffer;
	gBuffers loadarrowtail_buffer;
	shader load_shader;
	label_text_store load_value_labels;

	void set_load_arrowhead_vertices(float* load_arrowhead_vertices, unsigned int& load_arrowhead_v_index, 
		nodes_store* node, float load_angle, float load_value);
	void set_load_arrowhead_indices(unsigned int* load_arrowhead_vertex_indices, unsigned int& load_arrowhead_i_index);
	void set_load_arrowtail_vertices(float* load_arrowtail_vertices, unsigned int& load_arrowtail_v_index,
		nodes_store* node, float load_angle, float load_value, float load_max);
	void set_load_arrowtail_indices(unsigned int* load_arrowtail_vertex_indices, unsigned int& load_arrowtail_i_index);
};