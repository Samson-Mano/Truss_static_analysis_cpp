#pragma once
#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include "nodes_store.h"
#include "lines_store.h"
#include "buffers/gBuffers.h"
#include "buffers/Texture.h"
#include "shaders/shader.h"
#include "../../ImGui/stb_image.h"
#include <filesystem>
#include "bst_sorted_list.h"
#include "mconstraints.h"
#include "mloads.h"

class geom_store
{
public:
	geom_store();
	~geom_store();
	void create_geometry(const std::unordered_map<int, nodes_store>& nodeMap, std::unordered_map<int, lines_store>& lineMap);
	void paint_geometry();
	void updateWindowDimension(const int& window_width, const int& window_height);
	void deleteResources();
	void zoomfit_geometry();
	void pan_geometry(glm::vec2& transl);
	void zoom_geometry(float& z_scale);
	void set_nodal_loads(glm::vec2& loc, float& load_value, float& load_angle, bool is_add);
	void set_nodal_constraints(glm::vec2& loc, int& constraint_type, float& constraint_angle, bool is_add);
private:
	const float node_circle_radii = 0.005f;
	// Create an unordered_map to store nodes with ID as key
	std::unordered_map<int, nodes_store> nodeMap;
	// Create an unordered_map to store lines with ID as key
	std::unordered_map<int, lines_store> lineMap;
	// Constraint data store
	mconstraints constraintMap;
	// Load data store
	mloads loadMap;

	// Count
	unsigned int node_count;
	unsigned int line_count;
	// store the bounds of geometry
	glm::vec3 min_b; // (min_x, min_y,0)
	glm::vec3 max_b; // (max_x, max_y,0)
	glm::vec3 geom_bound; // Bound magnitude
	glm::vec3 center; // center of the geometry
	glm::mat4 modelMatrix; // Geometry model matrix
	float geom_scale; // Scale of the geometry

	// Screen transformations
	glm::mat4 panTranslation;
	float zoom_scale; // Zoom scale


	// Store the openGL buffers
	gBuffers line_buffer;
	gBuffers node_buffer;
	gBuffers constraint_buffer;
	gBuffers loadarrowhead_buffer;
	gBuffers loadarrowtail_buffer;
	Texture node_texture;
	Texture constraint_texture_pin;
	Texture constraint_texture_roller;

	shader node_shader;
	shader line_shader;
	shader constraint_shader;
	shader load_shader;

	// Functions to set the geometry
	int window_width;
	int window_height;
	bool is_geometry_loaded;
	bool is_geometry_set;
	void set_geometry();
	void set_line_vertices(float* line_vertices, unsigned int& line_v_index, nodes_store& node);
	void set_node_vertices(float* node_vertices, unsigned int& node_v_index, nodes_store& node);
	void set_node_indices(unsigned int* node_indices, unsigned int& node_i_index);
	void set_constraint_vertices(float* constraint_vertices, unsigned int& constraint_v_index, nodes_store* node, float constraint_angle, unsigned int constraint_type);
	void set_constraint_indices(unsigned int* constraint_vertex_indices, unsigned int& constraint_i_index);
	void set_load_arrowhead_vertices(float* load_arrowhead_vertices, unsigned int& load_arrowhead_v_index, nodes_store* node, float load_angle, float load_value);
	void set_load_arrowhead_indices(unsigned int* load_arrowhead_vertex_indices, unsigned int& load_arrowhead_i_index);
	void set_load_arrowtail_vertices(float* load_arrowtail_vertices, unsigned int& load_arrowtail_v_index, nodes_store* node, float load_angle, float load_value, float load_max);
	void set_load_arrowtail_indices(unsigned int* load_arrowtail_vertex_indices, unsigned int& load_arrowtail_i_index);

	std::pair<glm::vec3, glm::vec3> findMinMaxXY(const std::unordered_map<int, nodes_store>& nodeMap);
	glm::vec3 findGeometricCenter(const std::unordered_map<int, nodes_store>& nodeMap);
	void set_model_matrix();
	int is_node_hit(glm::vec2& loc);
	void update_constraint();
	void update_load();
};
