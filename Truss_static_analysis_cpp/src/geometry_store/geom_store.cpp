#include "geom_store.h"

geom_store::geom_store()
	: is_geometry_set(false),
	is_geometry_loaded(false),
	node_count(0), line_count(0),
	min_b(glm::vec3(0)),
	max_b(glm::vec3(0)),
	geom_bound(glm::vec3(0)), geom_scale(1.0f),
	center(glm::vec3(0)), window_width(0), window_height(0),
	line_buffer(), node_buffer(), node_texture(), node_sh(), model_sh()
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

	// Set the model matrix
	set_model_matrix();

	// Set the model to zoom fit (No pan, No rotation, No zoom scale)
	zoomfit_geometry();


	// Set the geometry
	// Define the line vertices of the model (3 node position & 3 color)
	const unsigned int line_vertex_count = 6 * node_count;
	float* line_vertices = new float[line_vertex_count];

	// Define the node vertices of the model (4 vertex (to form a triangle) for a node (2 position & 2 texture coordinate) 
	const unsigned int node_vertex_count = 4 * 8 * node_count;
	float* node_vertices = new float[node_vertex_count];

	unsigned int node_indices_count = 6 * node_count;
	unsigned int* node_vertex_indices = new unsigned int[node_indices_count];

	std::unordered_map<int, int> node_id_map;

	// Assuming you have an index variable to keep track of the array position
	unsigned int line_v_index = 0;
	unsigned int node_v_index = 0;
	unsigned int node_i_index = 0;
	int v_id = 0;

	for (auto& node : nodeMap)
	{
		// node_store
		// Add the node id to map
		node_id_map[node.first] = v_id;

		// Add the line point
		set_line_vertices(line_vertices, line_v_index, node.second);

		// Add 4 corner points for triangle in place of nodes
		set_node_vertices(node_vertices, node_v_index, node.second);
		// Add the indices
		set_node_indices(node_vertex_indices, node_i_index);
		v_id++;
	}

	// Define the indices of the lines of the model
	unsigned int line_indices_count = 2 * line_count;

	unsigned int* line_indices = new unsigned int[line_indices_count];
	unsigned int line_i_index = 0;

	for (const auto& line : lineMap)
	{
		// Add the node point
		line_indices[line_i_index + 0] = node_id_map[line.second.s_nd.node_id];
		line_indices[line_i_index + 1] = node_id_map[line.second.e_nd.node_id];

		line_i_index = line_i_index + 2;
	}

	unsigned int line_vertex_size = line_vertex_count * sizeof(float);

	VertexBufferLayout line_layout;
	line_layout.AddFloat(3);  // Position
	line_layout.AddFloat(3);  // Color

	// Create the Line buffers
	line_buffer.CreateBuffers((void*)line_vertices, line_vertex_size, (unsigned int*)line_indices, line_indices_count, line_layout);

	unsigned int node_vertex_size = node_vertex_count * sizeof(float);

	VertexBufferLayout node_layout;
	node_layout.AddFloat(3);  // Position
	node_layout.AddFloat(3);  // Color
	node_layout.AddFloat(2);  // Texture co-ordinate

	// Create the Node buffers
	node_buffer.CreateBuffers((void*)node_vertices, node_vertex_size, (unsigned int*)node_vertex_indices, node_indices_count, node_layout);

	// Create shader
	std::filesystem::path currentDirPath = std::filesystem::current_path();
	std::filesystem::path parentPath = currentDirPath.parent_path();
	std::filesystem::path shadersPath = parentPath / "Truss_static_analysis_cpp/Truss_static_analysis_cpp/src/geometry_store/shaders";
	std::string parentString = shadersPath.string();
	std::cout << "Parent path: " << parentString << std::endl;

	// Model shader
	model_sh.create_shader((shadersPath.string() + "/geom_vertex_shader.vert").c_str(),
		(shadersPath.string() + "/geom_frag_shader.frag").c_str());

	// Node shader
	node_sh.create_shader((shadersPath.string() + "/node_vertex_shader.vert").c_str(),
		(shadersPath.string() + "/node_frag_shader.frag").c_str());

	node_texture.LoadTexture((shadersPath.string() + "/node_texture2.png").c_str());
	node_texture.Bind();
	node_sh.setUniform("u_Texture", 0);

	// Geometry is set
	is_geometry_set = true;

	// Delete the Dynamic arrays
	delete[] line_vertices;
	delete[] node_vertices;
	delete[] node_vertex_indices;
	delete[] line_indices;
}

void geom_store::set_line_vertices(float* line_vertices, unsigned int& line_v_index, nodes_store& node)
{
	// Set the line vertices
	line_vertices[line_v_index + 0] = node.node_pt.x;
	line_vertices[line_v_index + 1] = node.node_pt.y;
	line_vertices[line_v_index + 2] = node.node_pt.z;

	// line default color
	line_vertices[line_v_index + 3] = node.default_color.x;
	line_vertices[line_v_index + 4] = node.default_color.y;
	line_vertices[line_v_index + 5] = node.default_color.z;

	// Increment
	line_v_index = line_v_index + 6;
}

void  geom_store::set_node_vertices(float* node_vertices, unsigned int& node_v_index, nodes_store& node)
{
	// Set the node vertices
	float node_size = 0.01f/geom_scale;

	// Set the node vertices Corner 1
	node_vertices[node_v_index + 0] = node.node_pt.x - node_size;
	node_vertices[node_v_index + 1] = node.node_pt.y - node_size;
	node_vertices[node_v_index + 2] = 0.0f;

	// Set the node color
	node_vertices[node_v_index + 3] = node.default_color.x;
	node_vertices[node_v_index + 4] = node.default_color.y * 0.0f;
	node_vertices[node_v_index + 5] = node.default_color.z;

	// Set the Texture co-ordinates
	node_vertices[node_v_index + 6] = 0.0f;
	node_vertices[node_v_index + 7] = 0.0f;

	// Increment
	node_v_index = node_v_index + 8;

	// Set the node vertices Corner 2
	node_vertices[node_v_index + 0] = node.node_pt.x + node_size;
	node_vertices[node_v_index + 1] = node.node_pt.y - node_size;
	node_vertices[node_v_index + 2] = 0.0f;

	// Set the node color
	node_vertices[node_v_index + 3] = node.default_color.x;
	node_vertices[node_v_index + 4] = node.default_color.y;
	node_vertices[node_v_index + 5] = node.default_color.z;

	// Set the Texture co-ordinates
	node_vertices[node_v_index + 6] = 1.0f;
	node_vertices[node_v_index + 7] = 0.0f;

	// Increment
	node_v_index = node_v_index + 8;

	// Set the node vertices Corner 3
	node_vertices[node_v_index + 0] = node.node_pt.x + node_size;
	node_vertices[node_v_index + 1] = node.node_pt.y + node_size;
	node_vertices[node_v_index + 2] = 0.0f;

	// Set the node color
	node_vertices[node_v_index + 3] = node.default_color.x;
	node_vertices[node_v_index + 4] = node.default_color.y;
	node_vertices[node_v_index + 5] = node.default_color.z;

	// Set the Texture co-ordinates
	node_vertices[node_v_index + 6] = 1.0f;
	node_vertices[node_v_index + 7] = 1.0f;

	// Increment
	node_v_index = node_v_index + 8;

	// Set the node vertices Corner 3
	node_vertices[node_v_index + 0] = node.node_pt.x - node_size;
	node_vertices[node_v_index + 1] = node.node_pt.y + node_size;
	node_vertices[node_v_index + 2] = 0.0f;

	// Set the node color
	node_vertices[node_v_index + 3] = node.default_color.x;
	node_vertices[node_v_index + 4] = node.default_color.y;
	node_vertices[node_v_index + 5] = node.default_color.z;

	// Set the Texture co-ordinates
	node_vertices[node_v_index + 6] = 0.0f;
	node_vertices[node_v_index + 7] = 1.0f;

	// Increment
	node_v_index = node_v_index + 8;
}

void geom_store::set_node_indices(unsigned int* node_indices, unsigned int& node_i_index)
{
	// Set the node indices
	unsigned int t_id = ((node_i_index / 6) * 4);
	// Triangle 0,1,2
	node_indices[node_i_index + 0] = t_id + 0;
	node_indices[node_i_index + 1] = t_id + 1;
	node_indices[node_i_index + 2] = t_id + 2;

	// Triangle 2,3,0
	node_indices[node_i_index + 3] = t_id + 2;
	node_indices[node_i_index + 4] = t_id + 3;
	node_indices[node_i_index + 5] = t_id + 0;

	// Increment
	node_i_index = node_i_index + 6;
}



void geom_store::paint_geometry()
{
	if (is_geometry_set == false)
		return;

	// Clean the back buffer and assign the new color to it
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Paint the Lines
	model_sh.Bind();
	line_buffer.Bind();
	glDrawElements(GL_LINES, 2 * line_count, GL_UNSIGNED_INT, 0);
	line_buffer.UnBind();
	model_sh.UnBind();

	// Paint the Nodes
	node_sh.Bind();
	node_buffer.Bind();
	// glPointSize(8.0f);
	glDrawElements(GL_TRIANGLES, 6 * node_count, GL_UNSIGNED_INT, 0);
	node_buffer.UnBind();
	node_sh.UnBind();

	//model_sh.UnBind();

	// GL.DrawElements(PrimitiveType.Points, this._point_indices.Length, DrawElementsType.UnsignedInt, 0);


}

void geom_store::set_model_matrix()
{
	// Set the model matrix for the model shader
	// Find the scale of the model (with 0.5 being the maximum used)
	int max_dim = window_width > window_height ? window_width : window_height;

	float normalized_screen_width = 1.8f * (float(window_width) / float(max_dim));
	float normalized_screen_height = 1.8f * (float(window_height) / float(max_dim));


	geom_scale = std::min(normalized_screen_width / geom_bound.x,
		normalized_screen_height / geom_bound.y);

	// Translation
	glm::vec3 geom_translation = glm::vec3(-1.0f * (max_b.x + min_b.x) * 0.5f * geom_scale,
		-1.0f * (max_b.y + min_b.y) * 0.5f * geom_scale,
		-1.0f * (max_b.z + min_b.z) * 0.5f * geom_scale);

	glm::mat4 g_transl = glm::translate(glm::mat4(1.0f), geom_translation);

	glm::mat4 modelMatrix = g_transl * glm::scale(glm::mat4(1.0f), glm::vec3(geom_scale));

	model_sh.setUniform("modelMatrix", modelMatrix, false);
	node_sh.setUniform("modelMatrix", modelMatrix, false);
}

void geom_store::updateWindowDimension(const int& window_width, const int& window_height)
{
	// Update the window dimension
	this->window_width = window_width;
	this->window_height = window_height;

	if (is_geometry_set == true)
	{
		// Update the model matrix
		set_model_matrix();
		// !! Zoom to fit operation during window resize is handled in mouse event class !!
	}
}

void geom_store::zoomfit_geometry()
{
	// Zoom Fit the geometry
	// Set the rotation matrix
	glm::mat4 rotationMatrix(1.0f);

	model_sh.setUniform("rotationMatrix", rotationMatrix, false);
	node_sh.setUniform("rotationMatrix", rotationMatrix, false);

	// Set the pan translation matrix
	glm::mat4 panTranslation(1.0f);

	model_sh.setUniform("panTranslation", panTranslation, false);
	node_sh.setUniform("panTranslation", panTranslation, false);

	// Set the zoom matrix
	float zoomscale = 1.0f;

	model_sh.setUniform("zoomscale", zoomscale);
	node_sh.setUniform("zoomscale", zoomscale);
}


void geom_store::pan_geometry(glm::vec2& transl)
{
	//// Pan the geometry
	//int max_dim = window_width > window_height ? window_width : window_height;

	//// Pan Translation
	//float x_transl = 2 * ((-transl.x) / float(max_dim));
	//float y_transl = 2 * ((-transl.y) / float(max_dim));

	// std::cout << "Pan translation " << x_transl << ", " << y_transl << std::endl;

	// Pan the geometry
	glm::mat4 panTranslation(1.0f);

	panTranslation[0][3] = -1.0f * transl.x;
	panTranslation[1][3] = transl.y;

	model_sh.setUniform("panTranslation", panTranslation, false);
	node_sh.setUniform("panTranslation", panTranslation, false);

}


void geom_store::zoom_geometry(float& z_scale)
{
	// Zoom the geometry
	model_sh.setUniform("zoomscale", z_scale);
	node_sh.setUniform("zoomscale", z_scale);
}