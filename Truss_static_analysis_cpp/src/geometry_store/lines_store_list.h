#pragma once
#include "nodes_store_list.h"
#include "../material_window.h"

struct lines_store
{
	int line_id; // ID of the line
	int material_id; // Material ID of the line member
	nodes_store startNode; // start node
	nodes_store endNode; // end node
	double member_force; // Result member force
	double member_stress; // Result member stress
};


class lines_store_list
{
public:
	unsigned int line_count = 0;
	std::unordered_map<int, lines_store> lineMap; // Create an unordered_map to store nodes with ID as key
	double max_displacement = 0.0;
	double max_resultant = 0.0;
	double max_memberforce = 0.0;
	double max_memberstress = 0.0;

	lines_store_list();
	~lines_store_list();
	void init(geom_parameters* geom_param_ptr);
	void add_line(int& line_id, const nodes_store& startNode, const nodes_store& endNode, int material_id);
	void set_buffer();
	void set_defl_buffer();
	void update_material_id_buffer();
	void paint_lines();
	void paint_line_ids();
	void paint_line_length();
	void paint_line_material_id();
	void paint_line_defl();
	int is_line_hit(glm::vec2& loc);
	void update_geometry_matrices(bool is_modelmatrix, bool is_pantranslation, bool is_zoomtranslation, bool set_transparency);
	void add_node_list(std::unordered_map<int, nodes_store>* model_nodes_ptr);
	void update_result_matrices(float defl_scale);
	void set_result_max(double max_displacement, double max_resultant,double max_member_force, double max_member_stress);
	void update_results(int& line_id, double youngs_mod, double cs_area);
private:
	std::unordered_map<int,nodes_store>* model_nodes_ptr;
	geom_parameters* geom_param_ptr;
	label_text_store line_id_labels;
	label_text_store line_length_labels;
	label_text_store material_id_labels;
	gBuffers line_buffer;
	gBuffers line_defl_buffer;
	shader line_shader;
	shader line_defl_shader;
	shader result_text_shader;

	bool isClickPointOnLine(const glm::vec2& clickPoint, const glm::vec2& lineStart, const glm::vec2& lineEnd, float threshold);
	void set_line_vertices(float* line_vertices, unsigned int& line_v_index,const nodes_store& node, int is_rslt);
	void set_line_indices(unsigned int* line_indices, unsigned int& line_i_index, std::unordered_map<int, int>& node_id_map, const lines_store& line);
};