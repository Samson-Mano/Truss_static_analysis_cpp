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
	line_mforce_labels.geom_param_ptr = geom_param_ptr;
	line_mstress_labels.geom_param_ptr = geom_param_ptr;

	// Delete all the lines
	line_count = 0;
	line_id_labels.delete_all();
	line_length_labels.delete_all();
	material_id_labels.delete_all();
	line_mforce_labels.delete_all();
	line_mstress_labels.delete_all();
	lineMap.clear();
}

void lines_store_list::add_node_list(std::unordered_map<int, nodes_store>* model_nodes_ptr)
{
	// Set the model nodes
	this->model_nodes_ptr = model_nodes_ptr;
}

void lines_store_list::add_line(int& line_id,const nodes_store startNode,const nodes_store endNode, int material_id)
{
	// Add the line to the list
	lines_store temp_line;
	temp_line.line_id = line_id;
	temp_line.startNode = startNode;
	temp_line.endNode = endNode;
	temp_line.material_id = material_id;
	temp_line.member_force = 0.0;
	temp_line.member_stress = 0.0;

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

	line_id_labels.add_text(temp_str.c_str(), line_mid_pt , glm::vec2(0), temp_color, line_angle, true);

	// Add the node coordinate label
	float line_length = static_cast<float>(sqrt(pow(end_pt.x - start_pt.x, 2) + pow(end_pt.y - start_pt.y, 2)));
	temp_str = std::to_string(line_length);

	line_length_labels.add_text(temp_str.c_str(), line_mid_pt, glm::vec2(0), temp_color, line_angle, false);
}

void lines_store_list::set_buffer()
{
	// Update the buffer
	// Define the line vertices of the model (2 node position, 2 deflection, 3 color, 1 to note result or not)
	const unsigned int line_vertex_count = 8 * (*model_nodes_ptr).size();
	float* line_vertices = new float[line_vertex_count];

	// Node ID map to keep track of the ids of the nodes
	std::unordered_map<int, int> node_id_map;
	unsigned int line_v_index = 0;
	unsigned int v_id = 0;

	lines_store line_null; // line null

	for (auto& node : (*model_nodes_ptr))
	{
		// node_store
		// Add the node id to map
		node_id_map[node.first] = v_id;

		// Add the line point
		set_line_vertices(line_vertices, line_v_index, node.second, line_null,0);

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
	line_layout.AddFloat(2);  // Position	
	line_layout.AddFloat(2);  // Deflection
	line_layout.AddFloat(3);  // Color
	line_layout.AddFloat(1);  // int to track deflection

	// Create the Line buffers
	line_buffer.CreateBuffers((void*)line_vertices, line_vertex_size, (unsigned int*)line_indices, line_indices_count, line_layout);


	// Delete the dynamic array
	delete[] line_vertices;
	delete[] line_indices;
	
	// Create shader
	std::filesystem::path shadersPath = geom_param_ptr->resourcePath;// / "src/geometry_store/shaders";

	// Line shader
	line_shader.create_shader((shadersPath.string() + "/src/geometry_store/shaders/geom_vertex_shader.vert").c_str(),
		(shadersPath.string() + "/src/geometry_store/shaders/geom_frag_shader.frag").c_str());

	// Set the buffers for the labels
	line_id_labels.set_buffers();
	line_length_labels.set_buffers();
}

void lines_store_list::set_defl_buffer()
{
	// Update the deflection buffer
	// Define the line vertices of the model (2 node position, 2 deflection, 3 color, 1 to note result or not)
	const unsigned int line_vertex_count = 8 * (*model_nodes_ptr).size();
	float* line_vertices = new float[line_vertex_count];

	// Node ID map to keep track of the ids of the nodes
	std::unordered_map<int, int> node_id_map;
	unsigned int line_v_index = 0;
	unsigned int v_id = 0;

	lines_store line_null; // line null

	for (auto& node : (*model_nodes_ptr))
	{
		// node_store
		// Add the node id to map
		node_id_map[node.first] = v_id;

		// Add the line point
		set_line_vertices(line_vertices, line_v_index, node.second, line_null, 1);

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
	line_layout.AddFloat(2);  // Position	
	line_layout.AddFloat(2);  // Deflection
	line_layout.AddFloat(3);  // Color
	line_layout.AddFloat(1);  // int to track deflection

	// Create the Line buffers
	line_defl_buffer.CreateBuffers((void*)line_vertices, line_vertex_size, (unsigned int*)line_indices, line_indices_count, line_layout);


	// Delete the dynamic array
	delete[] line_vertices;
	delete[] line_indices;

	// Create shader
	std::filesystem::path shadersPath = geom_param_ptr->resourcePath;// / "src/geometry_store/shaders";

	// Line shader
	line_defl_shader.create_shader((shadersPath.string() + "/src/geometry_store/shaders/geom_vertex_shader.vert").c_str(),
		(shadersPath.string() + "/src/geometry_store/shaders/geom_frag_shader.frag").c_str());

	//// Set the buffers for the labels
	//line_id_labels.set_buffers();
	//line_length_labels.set_buffers();
}

void lines_store_list::set_mforce_buffer()
{
	// Update the member force buffer
	// Define the line vertices of the model (2 node position, 2 deflection, 3 color, 1 to note result or not)
	const unsigned int line_vertex_count = 8 * 2 * line_count;
	float* line_vertices = new float[line_vertex_count];

	// Node ID map to keep track of the ids of the nodes
	std::unordered_map<int, int> node_id_map;
	unsigned int line_v_index = 0;
	unsigned int v_id = 0;

	// Define the indices of the lines of the model
	unsigned int line_indices_count = 2 * line_count;
	unsigned int* line_indices = new unsigned int[line_indices_count];

	nodes_store node_null; // node null

	unsigned int line_i_index = 0;
	// Add the line index
	for (const auto& line : lineMap)
	{
		// lines store
		lines_store ln = line.second;

		// Add the node id to map (Strat node)
		node_id_map[ln.startNode.node_id] = v_id;
		v_id++;

		// Add the node id to map (End node)
		node_id_map[ln.endNode.node_id] = v_id;
		v_id++;

		// Set the line vertices
		set_line_vertices(line_vertices, line_v_index, node_null,ln, 2);

		// Set the line indices
		set_line_indices(line_indices, line_i_index, node_id_map, ln);
	}

	unsigned int line_vertex_size = line_vertex_count * sizeof(float);

	VertexBufferLayout line_layout;
	line_layout.AddFloat(2);  // Position	
	line_layout.AddFloat(2);  // Deflection
	line_layout.AddFloat(3);  // Color
	line_layout.AddFloat(1);  // int to track deflection

	// Create the Line buffers
	line_mforce_buffer.CreateBuffers((void*)line_vertices, line_vertex_size, (unsigned int*)line_indices, line_indices_count, line_layout);

	// Delete the dynamic array
	delete[] line_vertices;
	delete[] line_indices;

	// Create shader
	std::filesystem::path shadersPath = geom_param_ptr->resourcePath;// / "src/geometry_store/shaders";

	// Create the result text buffers
	result_text_shader.create_shader((shadersPath.string() + "/src/geometry_store/shaders/resulttext_vert_shader.vert").c_str(),
		(shadersPath.string() + "/src/geometry_store/shaders/resulttext_frag_shader.frag").c_str());

	// Set texture uniform variables
	result_text_shader.setUniform("u_Texture", 0);

	// Set the buffers for the displacement labels
	line_mforce_labels.set_buffers();
	line_mstress_labels.set_buffers();

	// Line shader
	line_mforce_shader.create_shader((shadersPath.string() + "/src/geometry_store/shaders/geom_vertex_shader.vert").c_str(),
		(shadersPath.string() + "/src/geometry_store/shaders/geom_frag_shader.frag").c_str());

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
		material_id_labels.add_text(temp_str.c_str(), line_mid_pt, glm::vec2(0), temp_color,  line_angle, true);
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

void lines_store_list::paint_line_defl()
{
	// Paint the line Deflection
	line_defl_shader.Bind();
	line_defl_buffer.Bind();
	glDrawElements(GL_LINES, 2 * line_count, GL_UNSIGNED_INT, 0);
	line_defl_buffer.UnBind();
	line_defl_shader.UnBind();
}

void lines_store_list::paint_line_mforce()
{
	// Paint the line member force
	line_mforce_shader.Bind();
	line_mforce_buffer.Bind();
	glDrawElements(GL_LINES, 2 * line_count, GL_UNSIGNED_INT, 0);
	line_mforce_buffer.UnBind();
	line_mforce_shader.UnBind();
}

void lines_store_list::paint_line_mstress_values()
{
	// Paint the line member stress values
	result_text_shader.Bind();
	line_mstress_labels.paint_text();
	result_text_shader.UnBind();
}

void lines_store_list::paint_line_mforce_values()
{
	// Paint the line member force values
	result_text_shader.Bind();
	line_mforce_labels.paint_text();
	result_text_shader.UnBind();
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

void lines_store_list::set_line_vertices(float* line_vertices, unsigned int& line_v_index, const nodes_store& node, const lines_store& line, int is_rslt)
{

	if (is_rslt == 0)
	{
		// Set the line vertices
		line_vertices[line_v_index + 0] = node.node_pt.x;
		line_vertices[line_v_index + 1] = node.node_pt.y;

		// Set the line deflection value
		line_vertices[line_v_index + 2] = static_cast<float>(node.nodal_displ.x / max_displacement);
		line_vertices[line_v_index + 3] = static_cast<float>(node.nodal_displ.y / max_displacement);

		// line default color
		line_vertices[line_v_index + 4] = geom_param_ptr->geom_colors.line_color.x;
		line_vertices[line_v_index + 5] = geom_param_ptr->geom_colors.line_color.y;
		line_vertices[line_v_index + 6] = geom_param_ptr->geom_colors.line_color.z;

		// Integer to track whether values have result data 
		line_vertices[line_v_index + 7] = static_cast<float>(is_rslt);

		// Increment
		line_v_index = line_v_index + 8;
	}
	else if (is_rslt == 1)
	{
		// Set the line vertices
		line_vertices[line_v_index + 0] = node.node_pt.x;
		line_vertices[line_v_index + 1] = node.node_pt.y;

		// Set the line deflection value
		line_vertices[line_v_index + 2] = static_cast<float>(node.nodal_displ.x / max_displacement);
		line_vertices[line_v_index + 3] = static_cast<float>(node.nodal_displ.y / max_displacement);

		// Line displacement color
		line_vertices[line_v_index + 4] = node.node_contour_color.x;
		line_vertices[line_v_index + 5] = node.node_contour_color.y;
		line_vertices[line_v_index + 6] = node.node_contour_color.z;

		// Integer to track whether values have result data 
		line_vertices[line_v_index + 7] = static_cast<float>(is_rslt);

		// Increment
		line_v_index = line_v_index + 8;
	}
	else if (is_rslt == 2)
	{
		// Set the line vertices
		line_vertices[line_v_index + 0] = line.startNode.node_pt.x;
		line_vertices[line_v_index + 1] = line.startNode.node_pt.y;

		// Set the line deflection value
		line_vertices[line_v_index + 2] = static_cast<float>((*model_nodes_ptr)[line.startNode.node_id].nodal_displ.x / max_displacement);
		line_vertices[line_v_index + 3] = static_cast<float>((*model_nodes_ptr)[line.startNode.node_id].nodal_displ.y / max_displacement);

		// Line displacement color
		line_vertices[line_v_index + 4] = line.member_force_color.x;
		line_vertices[line_v_index + 5] = line.member_force_color.y;
		line_vertices[line_v_index + 6] = line.member_force_color.z;

		// Integer to track whether values have result data 
		line_vertices[line_v_index + 7] = static_cast<float>(is_rslt);

		// Increment
		line_v_index = line_v_index + 8;

		//_________________________________________________________________________________

		// Set the line vertices
		line_vertices[line_v_index + 0] = line.endNode.node_pt.x;
		line_vertices[line_v_index + 1] = line.endNode.node_pt.y;

		// Set the line deflection value
		line_vertices[line_v_index + 2] = static_cast<float>((*model_nodes_ptr)[line.endNode.node_id].nodal_displ.x / max_displacement);
		line_vertices[line_v_index + 3] = static_cast<float>((*model_nodes_ptr)[line.endNode.node_id].nodal_displ.y / max_displacement);

		// Line displacement color
		line_vertices[line_v_index + 4] = line.member_force_color.x;
		line_vertices[line_v_index + 5] = line.member_force_color.y;
		line_vertices[line_v_index + 6] = line.member_force_color.z;

		// Integer to track whether values have result data 
		line_vertices[line_v_index + 7] = static_cast<float>(is_rslt);

		// Increment
		line_v_index = line_v_index + 8;
	}
}

void lines_store_list::set_line_indices(unsigned int* line_indices, unsigned int& line_i_index, std::unordered_map<int, int>& node_id_map, const lines_store& line)
{
	// Add the line_indices
	line_indices[line_i_index + 0] = node_id_map[line.startNode.node_id];
	line_indices[line_i_index + 1] = node_id_map[line.endNode.node_id];

	line_i_index = line_i_index + 2;
}

void lines_store_list::update_geometry_matrices(bool is_modelmatrix, bool is_pantranslation, bool is_zoomtranslation, bool set_transparency)
{
	// Set Transparency
	if (set_transparency == true)
	{
		line_shader.setUniform("transparency", geom_param_ptr->geom_transparency);
		line_defl_shader.setUniform("transparency", 1.0f);
		line_mforce_shader.setUniform("transparency", 1.0f);
		result_text_shader.setUniform("transparency", 1.0f);
	}

	// Model Matrix
	if (is_modelmatrix == true)
	{
		line_shader.setUniform("geom_scale", geom_param_ptr->geom_scale);
		line_defl_shader.setUniform("geom_scale", geom_param_ptr->geom_scale);
		line_mforce_shader.setUniform("geom_scale", geom_param_ptr->geom_scale);
		result_text_shader.setUniform("geom_scale", geom_param_ptr->geom_scale);

		line_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
		line_defl_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
		line_mforce_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
		result_text_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
	}

	// Pan Translation
	if (is_pantranslation == true)
	{
		line_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
		line_defl_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
		line_mforce_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
		result_text_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
	}

	// Zoom Translation
	if (is_zoomtranslation == true)
	{
		line_shader.setUniform("zoomscale", geom_param_ptr->zoom_scale);
		line_defl_shader.setUniform("zoomscale", geom_param_ptr->zoom_scale);
		line_mforce_shader.setUniform("zoomscale", geom_param_ptr->zoom_scale);
		result_text_shader.setUniform("zoomscale", geom_param_ptr->zoom_scale);
	}
}


void lines_store_list::update_result_matrices(float defl_scale)
{
	// Update the deflection scale uniform
	line_defl_shader.setUniform("deflscale", defl_scale);
	line_mforce_shader.setUniform("deflscale", defl_scale);
	result_text_shader.setUniform("deflscale", defl_scale);
}

void lines_store_list::update_results(int& line_id, double member_stress, double member_force)
{
	// Update the member force and member stress
	lineMap[line_id].member_stress = member_stress;
	lineMap[line_id].member_force = member_force;
}

void lines_store_list::set_result_max(double max_displacement, double max_resultant, double max_memberstress, double max_memberforce)
{
	// set the maximum values 
	this->max_displacement = max_displacement;
	this->max_resultant = max_resultant;
	this->max_memberstress = max_memberstress;
	this->max_memberforce = max_memberforce;

	// Clear the member force/ member stress labels
	line_mforce_labels.delete_all();
	line_mstress_labels.delete_all();

	for (auto& ln_m : lineMap)
	{
		lines_store ln = ln_m.second;

		// Find the member force color
		float force_scale = static_cast<float>(std::abs(ln.member_force) / max_memberforce);
		glm::vec3 member_force_color = nodes_store_list::getContourColor(force_scale);

		lineMap[ln.line_id].member_force_color = member_force_color;

		// Find the member stress color
		float stress_scale = static_cast<float>(std::abs(ln.member_stress) / max_memberstress);
		glm::vec3 member_stress_color = nodes_store_list::getContourColor(stress_scale);

		lineMap[ln.line_id].member_stress_color = member_stress_color;

		//_____________________________________________________________________________________________________________
		// Find the line parameters for line member force/ stress labels
		glm::vec2 start_pt = ln.startNode.node_pt;
		glm::vec2 end_pt = ln.endNode.node_pt;

		// Calculate the midpoint of the line segment
		glm::vec2 line_mid_pt = glm::vec2((start_pt.x + end_pt.x) * 0.5f, (start_pt.y + end_pt.y) * 0.5f);

		float line_angle = atan2(end_pt.y - start_pt.y, end_pt.x - start_pt.x);

		// Mid Point Displacement
		glm::vec2 node_displ_start_node = glm::vec2((*model_nodes_ptr)[ln.startNode.node_id].nodal_displ.x / max_displacement,
			(*model_nodes_ptr)[ln.startNode.node_id].nodal_displ.y / max_displacement);
		glm::vec2 node_displ_end_node = glm::vec2((*model_nodes_ptr)[ln.endNode.node_id].nodal_displ.x / max_displacement,
			(*model_nodes_ptr)[ln.endNode.node_id].nodal_displ.y / max_displacement);

		glm::vec2 line_mid_pt_displ = glm::vec2((node_displ_start_node.x + node_displ_end_node.x) * 0.5f, (node_displ_start_node.y + node_displ_end_node.y) * 0.5f);


		// Set the member force label
		std::string temp_str = std::to_string(ln.member_force);

		line_mforce_labels.add_text(temp_str.c_str(), line_mid_pt, line_mid_pt_displ,
			member_force_color, line_angle, false);

		// Set the member stress label
		temp_str = std::to_string(ln.member_stress);

		line_mstress_labels.add_text(temp_str.c_str(), line_mid_pt, line_mid_pt_displ,
			member_stress_color, line_angle, false);
	}

}