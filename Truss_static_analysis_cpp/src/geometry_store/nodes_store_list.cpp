#include "nodes_store_list.h"

nodes_store_list::nodes_store_list()
{
	// Empty constructor
}

nodes_store_list::~nodes_store_list()
{
}

void nodes_store_list::init(geom_parameters* geom_param_ptr)
{
	this->geom_param_ptr = geom_param_ptr;
	node_id_labels.geom_param_ptr = geom_param_ptr;
	node_coord_labels.geom_param_ptr = geom_param_ptr;
	node_displ_labels.geom_param_ptr = geom_param_ptr;

	// Delete all the nodes
	node_count = 0;
	node_id_labels.delete_all();
	node_coord_labels.delete_all();
	node_displ_labels.delete_all();
	nodeMap.clear();
}

void nodes_store_list::add_node(const int& node_id, const glm::vec3& node_pt)
{
	// Add the node to the list
	nodes_store temp_node;
	temp_node.node_id = node_id;
	temp_node.node_pt = node_pt;
	temp_node.node_contour_color = glm::vec3(0.0f, 0.0f, 0.0f);
	temp_node.nodal_displ = glm::vec2(0.0f, 0.0f);
	temp_node.nodal_reaction = glm::vec2(0.0f, 0.0f);

	// Check whether the node_id is already there
	if (nodeMap.find(node_id) != nodeMap.end())
	{
		// Node ID already exist (do not add)
		return;
	}

	// Insert to the nodes
	nodeMap.insert({ node_id, temp_node });
	node_count++;

	//__________________________ Add the node labels
	glm::vec3 temp_color;
	std::string temp_str;

	// Add the node id Label
	temp_color = geom_param_ptr->geom_colors.node_color;
	temp_str = std::to_string(node_id);

	node_id_labels.add_text(temp_str.c_str(), node_pt,glm::vec2(0), temp_color, 0.0f, true);

	// Add the node coordinate label
	temp_str = "(" + std::to_string(node_pt.x) + ", " + std::to_string(node_pt.y) + ")";

	node_coord_labels.add_text(temp_str.c_str(), node_pt, glm::vec2(0), temp_color, 0.0f, false);
}

void nodes_store_list::set_buffer()
{
	// Define the node vertices of the model (4 vertex (to form a triangle) for a node (2 position, 2 center, 2 defl, 3 color  & 2 texture coordinate, 1 defl value) 
	const unsigned int node_vertex_count = 4 * 12 * node_count;
	float* node_vertices = new float[node_vertex_count];

	unsigned int node_indices_count = 6 * node_count; // 6 indices to form a quadrilateral
	unsigned int* node_vertex_indices = new unsigned int[node_indices_count];

	unsigned int node_v_index = 0;
	unsigned int node_i_index = 0;

	// Set the node vertices
	for (auto& node : nodeMap)
	{
		// Add 4 corner points for quadrilateral in place of nodes
		set_node_vertices(node_vertices, node_v_index, node.second,0);

		// Add the node indices
		set_node_indices(node_vertex_indices, node_i_index);
	}

	VertexBufferLayout node_layout;
	node_layout.AddFloat(2);  // Node quad Position
	node_layout.AddFloat(2);  // Node center
	node_layout.AddFloat(2);  // Node defl center
	node_layout.AddFloat(3);  // Color
	node_layout.AddFloat(2);  // Texture co-ordinate
	node_layout.AddFloat(1);  // int to track deflection

	unsigned int node_vertex_size = node_vertex_count * sizeof(float); // Size of the node_vertex

	// Create the Node Deflection buffers
	node_buffer.CreateBuffers((void*)node_vertices, node_vertex_size, (unsigned int*)node_vertex_indices, node_indices_count, node_layout);

	// Create shader
	std::filesystem::path shadersPath = geom_param_ptr->resourcePath;// / "src/geometry_store/shaders";

	// Node shader
	node_shader.create_shader((shadersPath.string() + "/src/geometry_store/shaders/node_vertex_shader.vert").c_str(),
		(shadersPath.string() + "/src/geometry_store/shaders/node_frag_shader.frag").c_str());


	node_texture.LoadTexture((shadersPath.string() + "/src/geometry_store/shaders/pic_3d_circle_paint.png").c_str());
	node_shader.setUniform("u_Texture", 0);

	// Set the buffers for the labels
	node_id_labels.set_buffers();
	node_coord_labels.set_buffers();

	// Delete the dynamic array
	delete[] node_vertices;
	delete[] node_vertex_indices;
}

void nodes_store_list::set_defl_buffer()
{
	// Define the node vertices of the model (4 vertex (to form a triangle) for a node (2 position, 2 center, 2 defl, 3 color  & 2 texture coordinate, 1 defl value) 
	const unsigned int node_vertex_count = 4 * 12 * node_count;
	float* node_vertices = new float[node_vertex_count];

	unsigned int node_indices_count = 6 * node_count; // 6 indices to form a quadrilateral
	unsigned int* node_vertex_indices = new unsigned int[node_indices_count];

	unsigned int node_v_index = 0;
	unsigned int node_i_index = 0;

	// Set the node vertices
	for (auto& node : nodeMap)
	{
		// Add 4 corner points for quadrilateral in place of nodes
		set_node_vertices(node_vertices, node_v_index, node.second,1);

		// Add the node indices
		set_node_indices(node_vertex_indices, node_i_index);
	}

	VertexBufferLayout node_layout;
	node_layout.AddFloat(2);  // Node quad Position
	node_layout.AddFloat(2);  // Node center
	node_layout.AddFloat(2);  // Node defl center
	node_layout.AddFloat(3);  // Color
	node_layout.AddFloat(2);  // Texture co-ordinate
	node_layout.AddFloat(1);  // int to track deflection

	unsigned int node_vertex_size = node_vertex_count * sizeof(float); // Size of the node_vertex

	// Create the Node buffers
	node_defl_buffer.CreateBuffers((void*)node_vertices, node_vertex_size, (unsigned int*)node_vertex_indices, node_indices_count, node_layout);

	// Create shader
	std::filesystem::path shadersPath = geom_param_ptr->resourcePath;// / "src/geometry_store/shaders";

	// Node shader
	node_defl_shader.create_shader((shadersPath.string() + "/src/geometry_store/shaders/node_vertex_shader.vert").c_str(),
		(shadersPath.string() + "/src/geometry_store/shaders/node_frag_shader.frag").c_str());

	node_defl_texture.LoadTexture((shadersPath.string() + "/src/geometry_store/shaders/pic_3d_circle_paint.png").c_str());
	node_defl_shader.setUniform("u_Texture", 0);

	// Create the result text buffers
	result_text_shader.create_shader((shadersPath.string() + "/src/geometry_store/shaders/resulttext_vert_shader.vert").c_str(),
		(shadersPath.string() + "/src/geometry_store/shaders/resulttext_frag_shader.frag").c_str());

	// Set texture uniform variables
	result_text_shader.setUniform("u_Texture", 0);

	// Set the buffers for the displacement labels
	node_displ_labels.set_buffers();

	// Delete the dynamic array
	delete[] node_vertices;
	delete[] node_vertex_indices;

}


void nodes_store_list::paint_nodes()
{
	// Paint the nodes
	node_shader.Bind();
	node_buffer.Bind();
	node_texture.Bind();
	glDrawElements(GL_TRIANGLES, 6 * node_count, GL_UNSIGNED_INT, 0);
	node_texture.UnBind();
	node_buffer.UnBind();
	node_shader.UnBind();
}

void nodes_store_list::paint_node_ids()
{
	// Paint the node id labels
	node_id_labels.paint_text();
}

void nodes_store_list::paint_node_coords()
{
	// Paint the node coordinate labels
	node_coord_labels.paint_text();
}

void nodes_store_list::paint_nodes_defl()
{
	// paint the nodes with deflection
	node_defl_shader.Bind();
	node_defl_buffer.Bind();
	node_defl_texture.Bind();
	glDrawElements(GL_TRIANGLES, 6 * node_count, GL_UNSIGNED_INT, 0);
	node_defl_texture.UnBind();
	node_defl_buffer.UnBind();
	node_defl_shader.UnBind();
}

void nodes_store_list::paint_nodes_defl_values()
{
	// Paint the nodes deflection values
	result_text_shader.Bind();
	node_displ_labels.paint_text();
	result_text_shader.UnBind();
}

int nodes_store_list::is_node_hit(glm::vec2& loc)
{
	// Return the node id of node which is clicked
	// Covert mouse location to screen location
	int max_dim = geom_param_ptr->window_width > geom_param_ptr->window_height ? geom_param_ptr->window_width : geom_param_ptr->window_height;

	// Transform the mouse location to openGL screen coordinates
	float screen_x = 2.0f * ((loc.x - (geom_param_ptr->window_width * 0.5f)) / max_dim);
	float screen_y = 2.0f * (((geom_param_ptr->window_height * 0.5f) - loc.y) / max_dim);


	// Nodal location
	glm::mat4 scaling_matrix = glm::mat4(1.0) * geom_param_ptr->zoom_scale;
	scaling_matrix[3][3] = 1.0f;

	glm::mat4 scaledModelMatrix = scaling_matrix * geom_param_ptr->modelMatrix;

	// Loop through all nodes in map and update min and max values
	for (auto it = nodeMap.begin(); it != nodeMap.end(); ++it)
	{
		const auto& node = it->second.node_pt;
		glm::vec4 finalPosition = scaledModelMatrix * glm::vec4(node.x, node.y, 0, 1.0f) * geom_param_ptr->panTranslation;

		float node_position_x = finalPosition.x;
		float node_position_y = finalPosition.y;

		if ((((node_position_x - screen_x) * (node_position_x - screen_x)) +
			((node_position_y - screen_y) * (node_position_y - screen_y))) < (16 * geom_param_ptr->node_circle_radii * geom_param_ptr->node_circle_radii))
		{
			// Return the id of the node
			// 4 x Radius is the threshold of hit (2 * Diameter)
			return it->first;
		}
	}

	// None found
	return -1;
}

void  nodes_store_list::set_node_vertices(float* node_vertices, unsigned int& node_v_index, nodes_store& node, int is_rslt)
{
	// Set the node vertices
	float node_size = geom_param_ptr->node_circle_radii / geom_param_ptr->geom_scale;

	// Set the node vertices Corner 1
	node_vertices[node_v_index + 0] = node.node_pt.x - node_size;
	node_vertices[node_v_index + 1] = node.node_pt.y - node_size;

	// node center
	node_vertices[node_v_index + 2] = node.node_pt.x;
	node_vertices[node_v_index + 3] = node.node_pt.y;

	// Set the node deflection center
	node_vertices[node_v_index + 4] = static_cast<float>(node.nodal_displ.x / max_displacement);
	node_vertices[node_v_index + 5] = static_cast<float>(node.nodal_displ.y / max_displacement);

	// Set the node color
	if(is_rslt == 0)
	{
		node_vertices[node_v_index + 6] = geom_param_ptr->geom_colors.node_color.x;
		node_vertices[node_v_index + 7] = geom_param_ptr->geom_colors.node_color.y;
		node_vertices[node_v_index + 8] = geom_param_ptr->geom_colors.node_color.z;
	}
	else
	{
		node_vertices[node_v_index + 6] = node.node_contour_color.x;
		node_vertices[node_v_index + 7] = node.node_contour_color.y;
		node_vertices[node_v_index + 8] = node.node_contour_color.z;
	}

	// Set the Texture co-ordinates
	node_vertices[node_v_index + 9] = 0.0f;
	node_vertices[node_v_index + 10] = 0.0f;

	// Integer to track whether values have result data 
	node_vertices[node_v_index + 11] = static_cast<float>(is_rslt);

	// Increment
	node_v_index = node_v_index + 12;

	// Set the node vertices Corner 2
	node_vertices[node_v_index + 0] = node.node_pt.x + node_size;
	node_vertices[node_v_index + 1] = node.node_pt.y - node_size;

	// node center
	node_vertices[node_v_index + 2] = node.node_pt.x;
	node_vertices[node_v_index + 3] = node.node_pt.y;

	// Set the node deflection center
	node_vertices[node_v_index + 4] = static_cast<float>(node.nodal_displ.x / max_displacement);
	node_vertices[node_v_index + 5] = static_cast<float>(node.nodal_displ.y / max_displacement);

	// Set the node color
	if (is_rslt == 0)
	{
		node_vertices[node_v_index + 6] = geom_param_ptr->geom_colors.node_color.x;
		node_vertices[node_v_index + 7] = geom_param_ptr->geom_colors.node_color.y;
		node_vertices[node_v_index + 8] = geom_param_ptr->geom_colors.node_color.z;
	}
	else
	{
		node_vertices[node_v_index + 6] = node.node_contour_color.x;
		node_vertices[node_v_index + 7] = node.node_contour_color.y;
		node_vertices[node_v_index + 8] = node.node_contour_color.z;
	}

	// Set the Texture co-ordinates
	node_vertices[node_v_index + 9] = 1.0f;
	node_vertices[node_v_index + 10] = 0.0f;

	// Integer to track whether values have result data 
	node_vertices[node_v_index + 11] = static_cast<float>(is_rslt);

	// Increment
	node_v_index = node_v_index + 12;

	// Set the node vertices Corner 3
	node_vertices[node_v_index + 0] = node.node_pt.x + node_size;
	node_vertices[node_v_index + 1] = node.node_pt.y + node_size;

	// node center
	node_vertices[node_v_index + 2] = node.node_pt.x;
	node_vertices[node_v_index + 3] = node.node_pt.y;

	// Set the node deflection center
	node_vertices[node_v_index + 4] = static_cast<float>(node.nodal_displ.x / max_displacement);
	node_vertices[node_v_index + 5] = static_cast<float>(node.nodal_displ.y / max_displacement);

	// Set the node color
	if (is_rslt == 0)
	{
		node_vertices[node_v_index + 6] = geom_param_ptr->geom_colors.node_color.x;
		node_vertices[node_v_index + 7] = geom_param_ptr->geom_colors.node_color.y;
		node_vertices[node_v_index + 8] = geom_param_ptr->geom_colors.node_color.z;
	}
	else
	{
		node_vertices[node_v_index + 6] = node.node_contour_color.x;
		node_vertices[node_v_index + 7] = node.node_contour_color.y;
		node_vertices[node_v_index + 8] = node.node_contour_color.z;
	}

	// Set the Texture co-ordinates
	node_vertices[node_v_index + 9] = 1.0f;
	node_vertices[node_v_index + 10] = 1.0f;

	// Integer to track whether values have result data 
	node_vertices[node_v_index + 11] = static_cast<float>(is_rslt);

	// Increment
	node_v_index = node_v_index + 12;

	// Set the node vertices Corner 4
	node_vertices[node_v_index + 0] = node.node_pt.x - node_size;
	node_vertices[node_v_index + 1] = node.node_pt.y + node_size;

	// node center
	node_vertices[node_v_index + 2] = node.node_pt.x;
	node_vertices[node_v_index + 3] = node.node_pt.y;

	// Set the node deflection center
	node_vertices[node_v_index + 4] = static_cast<float>(node.nodal_displ.x / max_displacement);
	node_vertices[node_v_index + 5] = static_cast<float>(node.nodal_displ.y / max_displacement);

	// Set the node color
	if (is_rslt == 0)
	{
		node_vertices[node_v_index + 6] = geom_param_ptr->geom_colors.node_color.x;
		node_vertices[node_v_index + 7] = geom_param_ptr->geom_colors.node_color.y;
		node_vertices[node_v_index + 8] = geom_param_ptr->geom_colors.node_color.z;
	}
	else
	{
		node_vertices[node_v_index + 6] = node.node_contour_color.x;
		node_vertices[node_v_index + 7] = node.node_contour_color.y;
		node_vertices[node_v_index + 8] = node.node_contour_color.z;
	}

	// Set the Texture co-ordinates
	node_vertices[node_v_index + 9] = 0.0f;
	node_vertices[node_v_index + 10] = 1.0f;

	// Integer to track whether values have result data 
	node_vertices[node_v_index + 11] = static_cast<float>(is_rslt);

	// Increment
	node_v_index = node_v_index + 12;
}

void nodes_store_list::set_node_indices(unsigned int* node_indices, unsigned int& node_i_index)
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

void nodes_store_list::update_geometry_matrices(bool is_modelmatrix, bool is_pantranslation, bool is_zoomtranslation, bool set_transparency)
{
	// Set Transparency
	if (set_transparency == true)
	{
		node_shader.setUniform("transparency", geom_param_ptr->geom_transparency);
		node_defl_shader.setUniform("transparency", 1.0f);
		result_text_shader.setUniform("transparency", 1.0f);
	}

	// Model Matrix
	if (is_modelmatrix == true)
	{
		node_shader.setUniform("geom_scale", geom_param_ptr->geom_scale);
		node_defl_shader.setUniform("geom_scale", geom_param_ptr->geom_scale);
		result_text_shader.setUniform("geom_scale", geom_param_ptr->geom_scale);

		node_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
		node_defl_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
		result_text_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
	}

	// Pan Translation
	if (is_pantranslation == true)
	{
		node_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
		node_defl_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
		result_text_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
	}

	// Zoom Translation
	if (is_zoomtranslation == true)
	{
		node_shader.setUniform("zoomscale", geom_param_ptr->zoom_scale);
		node_defl_shader.setUniform("zoomscale", geom_param_ptr->zoom_scale);
		result_text_shader.setUniform("zoomscale", geom_param_ptr->zoom_scale);
	}
}

void nodes_store_list::update_result_matrices(float defl_scale)
{
	// Update the deflection scale uniform
	node_defl_shader.setUniform("deflscale", defl_scale);
	result_text_shader.setUniform("deflscale", defl_scale);
}

void nodes_store_list::set_result_max(double max_displacement, double max_resultant)
{
	// Set the maximum displacement and maximum resultant
	this->max_displacement = max_displacement;
	this->max_resultant = max_resultant;

	// Clear the displacement lables
	node_displ_labels.delete_all();

	// Set the node contour color
	for (auto& nd_m : nodeMap)
	{
		int node_id = nd_m.first;

		// Set the node displ color
		double displ = std::sqrt(std::pow(nodeMap[node_id].nodal_displ.x, 2) +
			std::pow(nodeMap[node_id].nodal_displ.y, 2));
		double displ_scale = displ / max_displacement;

		// Set the Node Color
		nodeMap[node_id].node_contour_color = getContourColor(static_cast<float>(displ_scale));

		// Set the node displacement label
		std::string temp_str = "(" + std::to_string(nodeMap[node_id].nodal_displ.x) + ", " +
			std::to_string(nodeMap[node_id].nodal_displ.y) + ")";

		// Node Displacement
		glm::vec2 node_displ_offset = glm::vec2(nodeMap[node_id].nodal_displ.x / max_displacement, 
			nodeMap[node_id].nodal_displ.y / max_displacement);

		node_displ_labels.add_text(temp_str.c_str(), nodeMap[node_id].node_pt, node_displ_offset,
			nodeMap[node_id].node_contour_color, 0.0f, true);
	}
}

void nodes_store_list::update_results(int& node_id, double displ_x, double displ_y, double resultant_x, double resultant_y, double resultant_angle)
{
	// Update the Nodal results 
	nodeMap[node_id].nodal_displ = glm::vec2(displ_x, displ_y);
	nodeMap[node_id].nodal_reaction = glm::vec2(resultant_x, resultant_y);
	nodeMap[node_id].nodal_reaction_angle = resultant_angle;
}


glm::vec3 nodes_store_list::getContourColor(float value)
{
	// return the contour color based on the value (0 to 1)
	glm::vec3 color;
	float r, g, b;

	if (colormap_type == 0) 
	{
		// Jet color map
		r = glm::clamp(1.5f - glm::abs(4.0f * value - 3.0f), 0.0f, 1.0f);
		g = glm::clamp(1.5f - glm::abs(4.0f * value - 2.0f), 0.0f, 1.0f);
		b = glm::clamp(1.5f - glm::abs(4.0f * value - 1.0f), 0.0f, 1.0f);
	}
	else 
	{
		// Rainbow color map
		float hue = value * 5.0f; // Scale the value to the range of 0 to 5
		float c = 1.0f;
		float x = c * (1.0f - glm::abs(glm::mod(hue / 2.0f, 1.0f) - 1.0f));
		float m = 0.0f;

		if (hue >= 0 && hue < 1) {
			r = c;
			g = x;
			b = m;
		}
		else if (hue >= 1 && hue < 2) {
			r = x;
			g = c;
			b = m;
		}
		else if (hue >= 2 && hue < 3) {
			r = m;
			g = c;
			b = x;
		}
		else if (hue >= 3 && hue < 4) {
			r = m;
			g = x;
			b = c;
		}
		else {
			r = x;
			g = m;
			b = c;
		}
	}

	color = glm::vec3(r, g, b);
	return color;

}