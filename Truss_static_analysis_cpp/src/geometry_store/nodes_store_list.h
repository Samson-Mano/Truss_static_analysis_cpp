#pragma once
#include <glm/vec3.hpp>
#include <unordered_map>
#include "buffers/gBuffers.h"
#include "shaders/shader.h"
#include "buffers/Texture.h"
#include "geom_parameters.h"
#include "label_text_store.h"

struct nodes_store
{
	int node_id;
	glm::vec3 node_pt;
	glm::vec3 node_contour_color;
	glm::vec2 nodal_displ;
	glm::vec2 nodal_reaction;
};


class nodes_store_list
{
public:
	unsigned int node_count = 0;
	std::unordered_map<int, nodes_store> nodeMap; // Create an unordered_map to store nodes with ID as key
	double max_displacement = 0.0;
	double max_resultant = 0.0;

	nodes_store_list();
	~nodes_store_list();
	void init(geom_parameters* geom_param_ptr);
	void add_node(const int& node_id, const glm::vec3& node_pt);
	void set_buffer();
	void set_defl_buffer();
	void paint_nodes();
	void paint_node_ids();
	void paint_node_coords();
	void paint_nodes_defl();
	void paint_nodes_defl_values();
	int is_node_hit(glm::vec2& loc);
	void update_geometry_matrices(bool is_modelmatrix, bool is_pantranslation, bool is_zoomtranslation, bool set_transparency);
	void update_result_matrices(float defl_scale);
	void set_result_max(double max_displacement, double max_resultant);
	void update_results(int& node_id, double displ_x, double displ_y, double resultant_x, double resultant_y);

private:
	int colormap_type = 1; // 0 means jet and 1 means Rainbow
	geom_parameters* geom_param_ptr;
	label_text_store node_id_labels;
	label_text_store node_coord_labels;
	label_text_store node_displ_labels;
	gBuffers node_buffer;
	gBuffers node_defl_buffer;
	shader node_shader;
	shader node_defl_shader;
	shader result_text_shader;
	Texture node_texture;
	Texture node_defl_texture;

	void set_node_vertices(float* node_vertices, unsigned int& node_v_index, nodes_store& node,int is_rslt);
	void set_node_indices(unsigned int* node_indices, unsigned int& node_i_index);
	glm::vec3 getContourColor(float value);
};