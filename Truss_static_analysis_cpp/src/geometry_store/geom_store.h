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
private:
	// Create an unordered_map to store nodes with ID as key
	std::unordered_map<int, nodes_store> nodeMap;
	// Create an unordered_map to store lines with ID as key
	std::unordered_map<int, lines_store> lineMap;
	// Count
	unsigned int node_count;
	unsigned int line_count;
	// store the bounds of geometry
	glm::vec3 min_b; // (min_x, min_y,0)
	glm::vec3 max_b; // (max_x, max_y,0)
	glm::vec3 geom_bound; // Bound magnitude
	glm::vec3 center; // center of the geometry
	float geom_scale; // Scale of the geometry

	// Store the openGL buffers
	gBuffers line_buffer;
	gBuffers node_buffer;
	Texture node_texture;

	shader node_sh;
	shader model_sh;

	// Functions to set the geometry
	int window_width;
	int window_height;
	bool is_geometry_loaded;
	bool is_geometry_set;
	void set_geometry();
	void set_line_vertices(float* line_vertices, unsigned int& line_v_index, nodes_store& node);
	void set_node_vertices(float* node_vertices, unsigned int& node_v_index, nodes_store& node);
	void set_node_indices(unsigned int* node_indices, unsigned int& node_i_index);
;	std::pair<glm::vec3, glm::vec3> findMinMaxXY(const std::unordered_map<int, nodes_store>& nodeMap);
	glm::vec3 findGeometricCenter(const std::unordered_map<int, nodes_store>& nodeMap);
	void set_model_matrix();
};
