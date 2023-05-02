#include "geom_store.h"

geom_store::geom_store()
	: is_geometry_set(false),
	is_geometry_loaded(false),
	node_count(0), line_count(0),
	min_b(glm::vec3(0)),
	max_b(glm::vec3(0)),
	geom_bound(glm::vec3(0)),
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

	// Set the geometry
	// Define the vertices of the model
	const unsigned int vertex_count = 8 * node_count;
	unsigned int vertex_indices_count = node_count;

	float* vertices = new float[vertex_count]; //dynamic array
	unsigned int* vertex_indices = new unsigned int[vertex_indices_count];

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

		// Node default color
		vertices[index + 3] = node.second.default_color.x;
		vertices[index + 4] = node.second.default_color.y;
		vertices[index + 5] = node.second.default_color.z;

		// Node Texture
		vertices[index + 6] = 0.0f;
		vertices[index + 7] = 0.0f;

		vertex_indices[v_id] = v_id;
		v_id++;
		index = index + 8;
	}

	// Define the indices of the lines of the model
	unsigned int line_indices_count = 2 * line_count;

	unsigned int* line_indices = new unsigned int[line_indices_count];
	index = 0;

	for (const auto& line : lineMap)
	{
		// Add the node point
		line_indices[index + 0] = node_id_map[line.second.s_nd.node_id];
		line_indices[index + 1] = node_id_map[line.second.e_nd.node_id];

		index = index + 2;
	}

	unsigned int vertex_size = vertex_count * sizeof(float);

	VertexBufferLayout layout;
	layout.AddFloat(3);  // Position
	layout.AddFloat(3);  // Color
	layout.AddFloat(2);  // Texture co-ordinate

	// Create the Line buffers
	line_buffer.CreateBuffers((void*)vertices, vertex_size, (unsigned int*)line_indices, line_indices_count, layout);

	// Create the Node buffers
	node_buffer.CreateBuffers((void*)vertices, vertex_size, (unsigned int*)vertex_indices, vertex_indices_count, layout);

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


	// Set the model matrix
	set_model_matrix();

	// Set the model to zoom fit (No pan, No rotation, No zoom scale)
	zoomfit_geometry();


	// Geometry is set
	is_geometry_set = true;

	// Delete the Dynamic arrays
	delete[] vertices;
	delete[] vertex_indices;
	delete[] line_indices;
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
	glPointSize(8.0f);
	glDrawElements(GL_POINTS, node_count, GL_UNSIGNED_INT, 0);
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


	float geom_scale = std::min(normalized_screen_width / geom_bound.x,
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

	panTranslation[0][3] = -1.0f*transl.x;
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