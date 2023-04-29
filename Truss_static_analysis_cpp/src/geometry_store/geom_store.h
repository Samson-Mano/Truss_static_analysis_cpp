#pragma once
#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include "nodes_store.h"
#include "lines_store.h"
#include "buffers/VertexBuffer.h"
#include "buffers/IndexBuffer.h"
#include "buffers/VertexArray.h"
#include "shaders/shader.h"

class geom_store
{
public:
	geom_store();
	~geom_store();
	void create_geometry(const std::unordered_map<int, nodes_store>& nodeMap, std::unordered_map<int, lines_store>& lineMap);
	void paint_geometry();
	void deleteResources();
private:
	// Create an unordered_map to store nodes with ID as key
	std::unordered_map<int, nodes_store> nodeMap;
	// Create an unordered_map to store lines with ID as key
	std::unordered_map<int, lines_store> lineMap;
	// store the bounds of geometry
	glm::vec3 min_b; // (min_x, min_y,0)
	glm::vec3 max_b; // (max_x, max_y,0)
	glm::vec3 geom_bound; // Bound magnitude
	glm::vec3 center; // center of the geometry

	// Store the openGL buffers
	unsigned int shaderProgram;
	VertexBuffer vbo;
	VertexArray vao;
	IndexBuffer ibo;
	
	// Functions to set the geometry
	bool is_geometry_loaded;
	bool is_geometry_set;
	void set_geometry();
	std::pair<glm::vec3, glm::vec3> findMinMaxXY(const std::unordered_map<int, nodes_store>& nodeMap);
	glm::vec3 findGeometricCenter(const std::unordered_map<int, nodes_store>& nodeMap);
};
