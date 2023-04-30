#include "geom_store.h"

geom_store::geom_store()
	: is_geometry_set(false),
	is_geometry_loaded(false),
	node_count(0), line_count(0),
	min_b(glm::vec3(0)),
	max_b(glm::vec3(0)),
	geom_bound(glm::vec3(0)),
	center(glm::vec3(0)),
	vao(), vbo(), ibo(), sh(), model_sh()
{
	// Empty constructor
}

void geom_store::create_geometry(const std::unordered_map<int, nodes_store>& nodeMap,
	std::unordered_map<int, lines_store>& lineMap)
{
	// Constructor
	this->nodeMap = nodeMap;
	this->lineMap = lineMap;

	// Set the number of nodes and lines
	node_count = static_cast<unsigned int>(nodeMap.size());
	line_count = static_cast<unsigned int>(lineMap.size());

	// Set the boundary of the geometry
	std::pair<glm::vec3, glm::vec3> result = findMinMaxXY(nodeMap);
	min_b = result.first;
	max_b = result.second;
	geom_bound = max_b - min_b;

	// Set the center of the geometry
	center = findGeometricCenter(nodeMap);

	// Geometry is loaded
	is_geometry_loaded = true;

	set_geometry();
}

geom_store::~geom_store()
{
	// Destructor
	deleteResources();
}

void geom_store::deleteResources()
{
	is_geometry_loaded = false;
	is_geometry_set = false;
	// Call the destructor for each nodes_store object in the nodeMap
	for (auto& node : nodeMap) {
		node.second.~nodes_store();
	}
	// Call the destructor for each lines_store object in the lineMap
	for (auto& line : lineMap) {
		line.second.~lines_store();
	}
	// Clear the nodeMap and lineMap
	nodeMap.clear();
	lineMap.clear();
}

std::pair<glm::vec3, glm::vec3> geom_store::findMinMaxXY(const std::unordered_map<int, nodes_store>& nodeMap)
{
	// Initialize min and max values to first node in map
	auto firstNode = nodeMap.begin()->second.node_pt;
	glm::vec3 minXY = glm::vec3(firstNode.x, firstNode.y, firstNode.z);
	glm::vec3 maxXY = minXY;

	// Loop through all nodes in map and update min and max values
	for (auto it = nodeMap.begin(); it != nodeMap.end(); ++it)
	{
		const auto& node = it->second.node_pt;
		if (node.x < minXY.x)
		{
			minXY.x = node.x;
		}
		if (node.y < minXY.y)
		{
			minXY.y = node.y;
		}
		if (node.x > maxXY.x)
		{
			maxXY.x = node.x;
		}
		if (node.y > maxXY.y)
		{
			maxXY.y = node.y;
		}
	}

	// Return pair of min and max values
	return { minXY, maxXY };
}


glm::vec3 geom_store::findGeometricCenter(const std::unordered_map<int, nodes_store>& nodeMap)
{
	// Function returns the geometric center of the nodes
	// Initialize the sum with zero
	glm::vec3 sum(0);

	// Sum the points
	for (auto it = nodeMap.begin(); it != nodeMap.end(); ++it)
	{
		sum += it->second.node_pt;
	}
	return sum / static_cast<float>(nodeMap.size());
}

void geom_store::set_geometry()
{
	if (is_geometry_loaded == false)
		return;

	// Set the geometry
	// Define the vertices of the model
	const unsigned int vertex_size = 6 * node_count;

	float* vertices = new float[vertex_size]; //dynamic array
	std::unordered_map<int, int> node_id_map;

	// Assuming you have an index variable to keep track of the array position
	int index = 0;
	int v_id = 0;

	for (const auto& node : nodeMap)
	{
		// node_store
		// Add the node id to map
		node_id_map[node.first] = v_id;
		// Add the node point
		vertices[index + 0] = node.second.node_pt.x;
		vertices[index + 1] = node.second.node_pt.y;
		vertices[index + 2] = node.second.node_pt.z;

		vertices[index + 3] = node.second.default_color.x;
		vertices[index + 4] = node.second.default_color.y;
		vertices[index + 5] = node.second.default_color.z;

		v_id++;
		index = index + 6;
	}

	// Define the indices of the lines of the model
	const int indices_size = 2 * line_count;

	unsigned int* indices = new unsigned int[indices_size];
	index = 0;

	for (const auto& line : lineMap)
	{
		// Add the node point
		indices[index + 0] = node_id_map[line.second.s_nd.node_id];
		indices[index + 1] = node_id_map[line.second.e_nd.node_id];

		index = index + 2;
	}

	vao.createVertexArray();

	// Vertex buffer (vertices and number of vertices * sizeof(float)
	vbo.createVertexBuffer((void*)vertices, vertex_size * sizeof(float));

	// Index buffer (indices and number of indices)
	ibo.createIndexBuffer((unsigned int*)indices, indices_size);

	VertexBufferLayout layout;
	layout.AddFloat(3);  // Position
	layout.AddFloat(3);  // Color

	vao.AddBuffer(vbo, layout);

	// Create shader
	model_sh.create_shader("C:/Users/HFXMSZ/OneDrive - LR/Documents/Programming/Other programs/Cpp_projects/Truss_static_analysis_cpp/Truss_static_analysis_cpp/src/geometry_store/shaders/geom_vertex_shader.vert",
		"C:/Users/HFXMSZ/OneDrive - LR/Documents/Programming/Other programs/Cpp_projects/Truss_static_analysis_cpp/Truss_static_analysis_cpp/src/geometry_store/shaders/geom_frag_shader.frag");

	// Set the model matrix
	set_model_matrix(model_sh);

	// Set the rotation matrix
	glm::mat4 rotationMatrix(1.0f);

	model_sh.setUniform("rotationMatrix", rotationMatrix, false);

	glm::mat4 panTranslation(1.0f);

	model_sh.setUniform("panTranslation", panTranslation, false);

	float zoomscale = 1.0f;

	model_sh.setUniform("zoomscale", zoomscale);

	// Geometry is set
	is_geometry_set = true;

	// Delete the Dynamic arrays
	delete[] vertices;
	delete[] indices;
}

void geom_store::paint_geometry()
{
	if (is_geometry_set == false)
		return;

	// Clean the back buffer and assign the new color to it
	glClear(GL_COLOR_BUFFER_BIT);

	model_sh.Bind();
	vao.Bind();
	ibo.Bind();

	glDrawElements(GL_LINES, 2 * line_count, GL_UNSIGNED_INT, 0);

	model_sh.UnBind();
	vao.UnBind();
	ibo.UnBind();
}

void geom_store::set_model_matrix(shader& sh)
{
	// Set the model matrix for the model shader
	// Find the scale of the model (with 0.5 being the maximum used)

	float geom_scale = 0.8f / std::max(geom_bound.x, geom_bound.y);

	// Translation
	glm::vec3 geom_translation = glm::vec3(-1.0f * (max_b.x + min_b.x) * 0.5f * geom_scale,
		-1.0f * (max_b.y + min_b.y) * 0.5f * geom_scale,
		-1.0f * (max_b.z + min_b.z) * 0.5f * geom_scale);

	glm::mat4 g_transl = glm::translate(glm::mat4(1.0f), geom_translation);

	glm::mat4 modelMatrix = g_transl * glm::scale(glm::mat4(1.0f), glm::vec3(geom_scale));

	model_sh.setUniform("modelMatrix", modelMatrix, false);
}

/*

float vertices[] = {
		// Position          // Color
		-0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
		 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
	};

	// Define the indices of the triangle
	unsigned int indices[] = {
		0, 1, 2
	};

	vao.createVertexArray();
	vbo.createVertexBuffer(vertices, 3 * 6 * sizeof(float));

	// Create an index buffer object and bind it to the vertex array object
	ibo.createIndexBuffer(indices, 3);

	VertexBufferLayout layout;
	layout.AddFloat(3);  // Position
	layout.AddFloat(3);  // Color

	vao.AddBuffer(vbo, layout);


*/