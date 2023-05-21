#include "lines_store_list.h"

lines_store_list::lines_store_list()
{
	// Empty constructor
}

lines_store_list::~lines_store_list()
{
}

void lines_store_list::init(geom_parameters* geom_param_ptr)
{
	this->geom_param_ptr = geom_param_ptr;
	line_id_labels.geom_param_ptr = geom_param_ptr;
	line_length_labels.geom_param_ptr = geom_param_ptr;
	material_id_labels.geom_param_ptr = geom_param_ptr;

	// Delete all the lines
	line_count = 0;
	line_id_labels.delete_all();
	line_length_labels.delete_all();
	material_id_labels.delete_all();
	lineMap.clear();
}

void lines_store_list::add_node_list(std::unordered_map<int, nodes_store>* mode_nodes_ptr)
{
	// Set the model nodes
	this->mode_nodes_ptr = mode_nodes_ptr;
}

void lines_store_list::add_line(int& line_id, const nodes_store& startNode, const nodes_store& endNode, int material_id)
{
	// Add the line to the list
	lines_store temp_line;
	temp_line.line_id = line_id;
	temp_line.startNode = startNode;
	temp_line.endNode = endNode;
	temp_line.material_id = material_id;

	// Check whether the line_id is already there
	if (lineMap.find(line_id) != lineMap.end())
	{
		// Line ID already exist (do not add)
		return;
	}


	lineMap.insert({ line_id,temp_line });
	line_count++;

	//__________________________ Add the line labels
	glm::vec2 start_pt = temp_line.startNode.node_pt;
	glm::vec2 end_pt = temp_line.endNode.node_pt;

	glm::vec2 line_mid_pt = glm::vec2((start_pt.x + end_pt.x) * 0.5f, (start_pt.y + end_pt.y) * 0.5f);
	glm::vec3 temp_color;
	std::string temp_str;

	// Calculate the angle between the line segment and the x-axis
	float line_angle = atan2(end_pt.y - start_pt.y, end_pt.x - start_pt.x);

	// Add the node id Label
	temp_color = geom_param_ptr->geom_colors.line_color;
	temp_str = "[" + std::to_string(line_id) + "]";

	line_id_labels.add_text(temp_str.c_str(), line_mid_pt, temp_color, line_angle, true);

	// Add the node coordinate label
	float line_length = sqrt(pow(end_pt.x - start_pt.x, 2) + pow(end_pt.y - start_pt.y, 2));
	temp_str = std::to_string(line_length);

	line_length_labels.add_text(temp_str.c_str(), line_mid_pt, temp_color, line_angle, false);
}


void lines_store_list::set_buffer()
{
	// Update the buffer
		// Define the line vertices of the model (3 node position & 3 color)
	const unsigned int line_vertex_count = 6 * (*mode_nodes_ptr).size();
	float* line_vertices = new float[line_vertex_count];

	// Node ID map to keep track of the ids of the nodes
	std::unordered_map<int, int> node_id_map;
	unsigned int line_v_index = 0;
	unsigned int v_id = 0;

	for (auto& node : (*mode_nodes_ptr))
	{
		// node_store
		// Add the node id to map
		node_id_map[node.first] = v_id;

		// Add the line point
		set_line_vertices(line_vertices, line_v_index, node.second);

		v_id++;
	}

	// Define the indices of the lines of the model
	unsigned int line_indices_count = 2 * line_count;
	unsigned int* line_indices = new unsigned int[line_indices_count];

	unsigned int line_i_index = 0;
	// Add the line index
	for (const auto& line : lineMap)
	{
		set_line_indices(line_indices, line_i_index, node_id_map, line.second);

	}

	unsigned int line_vertex_size = line_vertex_count * sizeof(float);

	VertexBufferLayout line_layout;
	line_layout.AddFloat(3);  // Position
	line_layout.AddFloat(3);  // Color

	// Create the Line buffers
	line_buffer.CreateBuffers((void*)line_vertices, line_vertex_size, (unsigned int*)line_indices, line_indices_count, line_layout);


	// Delete the dynamic array
	delete[] line_vertices;
	delete[] line_indices;
	
	// Create shader
	std::filesystem::path currentDirPath = std::filesystem::current_path();
	std::filesystem::path parentPath = currentDirPath.parent_path();
	std::filesystem::path shadersPath = parentPath / "Truss_static_analysis_cpp/src/geometry_store/shaders";

	// Line shader
	line_shader.create_shader((shadersPath.string() + "/geom_vertex_shader.vert").c_str(),
		(shadersPath.string() + "/geom_frag_shader.frag").c_str());

	// Set the buffers for the labels
	line_id_labels.set_buffers();
	line_length_labels.set_buffers();
}

void lines_store_list::update_material_id_buffer()
{
	material_id_labels.delete_all();

	// Update the material ids
	glm::vec3 temp_color;
	std::string temp_str;

	for (auto& line : lineMap)
	{
		lines_store ln = line.second;

		// Add the line id
		glm::vec2 start_pt = ln.startNode.node_pt;
		glm::vec2 end_pt = ln.endNode.node_pt;

		// Calculate the midpoint of the line segment
		glm::vec2 line_mid_pt = glm::vec2((start_pt.x + end_pt.x) * 0.5f, (start_pt.y + end_pt.y) * 0.5f);

		float line_angle = atan2(end_pt.y - start_pt.y, end_pt.x - start_pt.x);

		// Add the material ID
		temp_color = material_window::get_standard_color(ln.material_id);
		temp_str = "                 M = " + std::to_string(ln.material_id);
		material_id_labels.add_text(temp_str.c_str(), line_mid_pt, temp_color,  line_angle, true);
	}

	 material_id_labels.set_buffers();
}

void lines_store_list::paint_lines()
{
	// Paint the lines
	line_shader.Bind();
	line_buffer.Bind();
	glDrawElements(GL_LINES, 2 * line_count, GL_UNSIGNED_INT, 0);
	line_buffer.UnBind();
	line_shader.UnBind();
}


void lines_store_list::paint_line_ids()
{
	// Paint the Line id labels
	line_id_labels.paint_text();
}

void lines_store_list::paint_line_length()
{
	// Paint the line length labels
	line_length_labels.paint_text();
}

void lines_store_list::paint_line_material_id()
{
	// Paint the line material ids
	material_id_labels.paint_text();
}

int lines_store_list::is_line_hit(glm::vec2& loc)
{
	// Return the line id of line which is clicked

		// Covert mouse location to screen location
	int max_dim = geom_param_ptr->window_width > geom_param_ptr->window_height ? geom_param_ptr->window_width : geom_param_ptr->window_height;

	// Transform the mouse location to openGL screen coordinates
	glm::vec2 screenPt = glm::vec2(2.0f * ((loc.x - (geom_param_ptr->window_width * 0.5f)) / max_dim),
		2.0f * (((geom_param_ptr->window_height * 0.5f) - loc.y) / max_dim));
	//float screen_x = 2.0f * ((loc.x - (window_width * 0.5f)) / max_dim);
	//float screen_y = 2.0f * (((window_height * 0.5f) - loc.y) / max_dim);


	// Nodal location
	glm::mat4 scaling_matrix = glm::mat4(1.0) * geom_param_ptr->zoom_scale;
	scaling_matrix[3][3] = 1.0f;

	glm::mat4 scaledModelMatrix = scaling_matrix * geom_param_ptr->modelMatrix;

	// Loop through all nodes in map and update min and max values
	for (auto it = lineMap.begin(); it != lineMap.end(); ++it)
	{
		const auto& s_node = it->second.startNode.node_pt;
		const auto& e_node = it->second.endNode.node_pt;

		glm::vec4 s_node_finalPosition = scaledModelMatrix * glm::vec4(s_node.x, s_node.y, 0, 1.0f) * geom_param_ptr->panTranslation;
		glm::vec4 e_node_finalPosition = scaledModelMatrix * glm::vec4(e_node.x, e_node.y, 0, 1.0f) * geom_param_ptr->panTranslation;

		// S & E Point 
		glm::vec2 spt = glm::vec2(s_node_finalPosition.x, s_node_finalPosition.y);
		glm::vec2 ept = glm::vec2(e_node_finalPosition.x, e_node_finalPosition.y);

		float threshold = 8 * geom_param_ptr->node_circle_radii;

		if (isClickPointOnLine(screenPt, spt, ept, threshold) == true)
		{
			// Return the Id of the line if hit == true
			return it->first;
		}
	}

	return -1;
}

bool lines_store_list::isClickPointOnLine(const glm::vec2& clickPoint, const glm::vec2& lineStart, const glm::vec2& lineEnd, float threshold)
{
	glm::vec2 lineDirection = lineEnd - lineStart;
	float lineLengthSq = glm::dot(lineDirection, lineDirection);

	glm::vec2 clickToLineStart = clickPoint - lineStart;
	float dotProduct = glm::dot(clickToLineStart, lineDirection);

	// Calculate the normalized projection of clickToLineStart onto the line
	glm::vec2 projection = (dotProduct / lineLengthSq) * lineDirection;

	// Calculate the squared normal distance between the click point and the line
	float normalDistanceSq = glm::dot(clickToLineStart - projection, clickToLineStart - projection);

	// Check if the click point is within the line segment's bounding box
	if (dotProduct >= 0.0f && dotProduct <= lineLengthSq)
	{
		// Check if the normal distance is less than or equal to the threshold
		if (normalDistanceSq <= threshold * threshold)
		{
			return true; // Click point is on the line segment
		}
	}

	return false; // Click point is not on the line segment
}

void lines_store_list::set_line_vertices(float* line_vertices, unsigned int& line_v_index, const nodes_store& node)
{
	// Set the line vertices
	line_vertices[line_v_index + 0] = node.node_pt.x;
	line_vertices[line_v_index + 1] = node.node_pt.y;
	line_vertices[line_v_index + 2] = node.node_pt.z;

	// line default color
	line_vertices[line_v_index + 3] = geom_param_ptr->geom_colors.line_color.x;
	line_vertices[line_v_index + 4] = geom_param_ptr->geom_colors.line_color.y;
	line_vertices[line_v_index + 5] = geom_param_ptr->geom_colors.line_color.z;

	// Increment
	line_v_index = line_v_index + 6;
}

void lines_store_list::set_line_indices(unsigned int* line_indices, unsigned int& line_i_index, std::unordered_map<int, int>& node_id_map, const lines_store& line)
{
	// Add the line_indices
	line_indices[line_i_index + 0] = node_id_map[line.startNode.node_id];
	line_indices[line_i_index + 1] = node_id_map[line.endNode.node_id];

	line_i_index = line_i_index + 2;
}

void lines_store_list::update_geometry_matrices(bool is_modelmatrix, bool is_pantranslation, bool is_zoomtranslation)
{
	// Model Matrix
	if (is_modelmatrix == true)
	{
		line_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
	}

	// Pan Translation
	if (is_pantranslation == true)
	{
		line_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
	}

	// Zoom Translation
	if (is_zoomtranslation == true)
	{
		line_shader.setUniform("zoomscale", geom_param_ptr->zoom_scale);
	}
}
