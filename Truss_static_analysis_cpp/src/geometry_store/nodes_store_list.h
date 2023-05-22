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
	glm::vec3 default_color; // Store the default color of this node
	glm::vec3 contour_color;
};


class nodes_store_list
{
public:
	unsigned int node_count = 0;
	std::unordered_map<int, nodes_store> nodeMap; // Create an unordered_map to store nodes with ID as key

	nodes_store_list();
	~nodes_store_list();
	void init(geom_parameters* geom_param_ptr);
	void add_node(const int& node_id, const glm::vec3& node_pt);
	void set_buffer();
	void paint_nodes();
	void paint_node_ids();
	void paint_node_coords();
	int is_node_hit(glm::vec2& loc);
	void update_geometry_matrices(bool is_modelmatrix, bool is_pantranslation, bool is_zoomtranslation, bool set_transparency);

private:
	geom_parameters* geom_param_ptr;
	label_text_store node_id_labels;
	label_text_store node_coord_labels;
	gBuffers node_buffer;
	shader node_shader;
	Texture node_texture;

	void set_node_vertices(float* node_vertices, unsigned int& node_v_index, nodes_store& node);
	void set_node_indices(unsigned int* node_indices, unsigned int& node_i_index);
};