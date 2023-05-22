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
	// Delete all the nodes
	node_count = 0;
	node_id_labels.delete_all();
	node_coord_labels.delete_all();
	nodeMap.clear();
}

void nodes_store_list::add_node(const int& node_id,const glm::vec3& node_pt)
{
	// Add the node to the list
	nodes_store temp_node;
	temp_node.node_id = node_id;
	temp_node.node_pt = node_pt;
	temp_node.default_color = glm::vec3(1.0f, 1.0f, 1.0f);
	temp_node.contour_color = glm::vec3(0.0f, 0.0f, 0.0f);

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

	node_id_labels.add_text(temp_str.c_str(), node_pt, temp_color, 0.0f,true);

	// Add the node coordinate label
	temp_str = "(" + std::to_string(node_pt.x) + ", " + std::to_string(node_pt.y) + ")";

	node_coord_labels.add_text(temp_str.c_str(), node_pt, temp_color, 0.0f, false);
}

void nodes_store_list::set_buffer()
{
	// Define the node vertices of the model (4 vertex (to form a triangle) for a node (3 position, 3 color, 3 center & 2 texture coordinate) 
	const unsigned int node_vertex_count = 4 * 11 * node_count;
	float* node_vertices = new float[node_vertex_count];

	unsigned int node_indices_count = 6 * node_count; // 6 indices to form a quadrilateral
	unsigned int* node_vertex_indices = new unsigned int[node_indices_count];

	unsigned int node_v_index = 0;
	unsigned int node_i_index = 0;

	// Set the node vertices
	for (auto& node : nodeMap)
	{
		// Add 4 corner points for quadrilateral in place of nodes
		set_node_vertices(node_vertices, node_v_index, node.second);

		// Add the node indices
		set_node_indices(node_vertex_indices, node_i_index);
	}

	VertexBufferLayout node_layout;
	node_layout.AddFloat(3);  // Position
	node_layout.AddFloat(3);  // Node center
	node_layout.AddFloat(3);  // Color
	node_layout.AddFloat(2);  // Texture co-ordinate

	unsigned int node_vertex_size = node_vertex_count * sizeof(float); // Size of the node_vertex

	// Create the Node buffers
	node_buffer.CreateBuffers((void*)node_vertices, node_vertex_size, (unsigned int*)node_vertex_indices, node_indices_count, node_layout);

	// Create shader
	std::filesystem::path currentDirPath = std::filesystem::current_path();
	std::filesystem::path parentPath = currentDirPath.parent_path();
	std::filesystem::path shadersPath = parentPath / "Truss_static_analysis_cpp/src/geometry_store/shaders";

	// Node shader
	node_shader.create_shader((shadersPath.string() + "/node_vertex_shader.vert").c_str(),
		(shadersPath.string() + "/node_frag_shader.frag").c_str());

	node_texture.LoadTexture((shadersPath.string() + "/pic_3d_circle_paint.png").c_str());
	node_shader.setUniform("u_Texture", 0);

	// Set the buffers for the labels
	node_id_labels.set_buffers();
	node_coord_labels.set_buffers();

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

void  nodes_store_list::set_node_vertices(float* node_vertices, unsigned int& node_v_index, nodes_store& node)
{
	// Set the node vertices
	float node_size = geom_param_ptr->node_circle_radii / geom_param_ptr->geom_scale;

	// Set the node vertices Corner 1
	node_vertices[node_v_index + 0] = node.node_pt.x - node_size;
	node_vertices[node_v_index + 1] = node.node_pt.y - node_size;
	node_vertices[node_v_index + 2] = 0.0f;

	// node center
	node_vertices[node_v_index + 3] = node.node_pt.x;
	node_vertices[node_v_index + 4] = node.node_pt.y;
	node_vertices[node_v_index + 5] = 0.0f;

	// Set the node color
	node_vertices[node_v_index + 6] = geom_param_ptr->geom_colors.node_color.x;
	node_vertices[node_v_index + 7] = geom_param_ptr->geom_colors.node_color.y;
	node_vertices[node_v_index + 8] = geom_param_ptr->geom_colors.node_color.z;

	// Set the Texture co-ordinates
	node_vertices[node_v_index +9] = 0.0f;
	node_vertices[node_v_index + 10] = 0.0f;

	// Increment
	node_v_index = node_v_index + 11;

	// Set the node vertices Corner 2
	node_vertices[node_v_index + 0] = node.node_pt.x + node_size;
	node_vertices[node_v_index + 1] = node.node_pt.y - node_size;
	node_vertices[node_v_index + 2] = 0.0f;

	// node center
	node_vertices[node_v_index + 3] = node.node_pt.x;
	node_vertices[node_v_index + 4] = node.node_pt.y;
	node_vertices[node_v_index + 5] = 0.0f;

	// Set the node color
	node_vertices[node_v_index + 6] = geom_param_ptr->geom_colors.node_color.x;
	node_vertices[node_v_index + 7] = geom_param_ptr->geom_colors.node_color.y;
	node_vertices[node_v_index + 8] = geom_param_ptr->geom_colors.node_color.z;

	// Set the Texture co-ordinates
	node_vertices[node_v_index + 9] = 1.0f;
	node_vertices[node_v_index + 10] = 0.0f;

	// Increment
	node_v_index = node_v_index + 11;

	// Set the node vertices Corner 3
	node_vertices[node_v_index + 0] = node.node_pt.x + node_size;
	node_vertices[node_v_index + 1] = node.node_pt.y + node_size;
	node_vertices[node_v_index + 2] = 0.0f;

	// node center
	node_vertices[node_v_index + 3] = node.node_pt.x;
	node_vertices[node_v_index + 4] = node.node_pt.y;
	node_vertices[node_v_index + 5] = 0.0f;

	// Set the node color
	node_vertices[node_v_index + 6] = geom_param_ptr->geom_colors.node_color.x;
	node_vertices[node_v_index + 7] = geom_param_ptr->geom_colors.node_color.y;
	node_vertices[node_v_index + 8] = geom_param_ptr->geom_colors.node_color.z;

	// Set the Texture co-ordinates
	node_vertices[node_v_index + 9] = 1.0f;
	node_vertices[node_v_index + 10] = 1.0f;

	// Increment
	node_v_index = node_v_index + 11;

	// Set the node vertices Corner 4
	node_vertices[node_v_index + 0] = node.node_pt.x - node_size;
	node_vertices[node_v_index + 1] = node.node_pt.y + node_size;
	node_vertices[node_v_index + 2] = 0.0f;

	// node center
	node_vertices[node_v_index + 3] = node.node_pt.x;
	node_vertices[node_v_index + 4] = node.node_pt.y;
	node_vertices[node_v_index + 5] = 0.0f;

	// Set the node color
	node_vertices[node_v_index + 6] = geom_param_ptr->geom_colors.node_color.x;
	node_vertices[node_v_index + 7] = geom_param_ptr->geom_colors.node_color.y;
	node_vertices[node_v_index + 8] = geom_param_ptr->geom_colors.node_color.z;

	// Set the Texture co-ordinates
	node_vertices[node_v_index + 9] = 0.0f;
	node_vertices[node_v_index + 10] = 1.0f;

	// Increment
	node_v_index = node_v_index + 11;
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

void nodes_store_list::update_geometry_matrices(bool is_modelmatrix, bool is_pantranslation, bool is_zoomtranslation,bool set_transparency)
{
	// Set Transparency
	if (set_transparency == true)
	{
		node_shader.setUniform("transparency", geom_param_ptr->geom_transparency);
	}

	// Model Matrix
	if (is_modelmatrix == true)
	{
		node_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
	}

	// Pan Translation
	if (is_pantranslation == true)
	{
		node_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
	}

	// Zoom Translation
	if (is_zoomtranslation == true)
	{
		node_shader.setUniform("zoomscale", geom_param_ptr->zoom_scale);
	}
}