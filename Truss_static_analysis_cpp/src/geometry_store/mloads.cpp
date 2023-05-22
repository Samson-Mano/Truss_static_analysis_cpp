#include "mloads.h"

mloads::mloads()
	:load_count(0)
{
	// Constructor
}

mloads::~mloads()
{
	// Destructor
}

void mloads::init(geom_parameters* geom_param_ptr)
{
	this->geom_param_ptr = geom_param_ptr;
	load_value_labels.geom_param_ptr = geom_param_ptr;

	// Delete all the Loads
	load_count = 0;
	load_value_labels.labels.clear();
	l_data.clear();
}

void mloads::add_load(int& node_id, nodes_store* node, float& load_value, float& load_angle)
{
	if (load_value == 0)
		return;

	// Add constraints
	load_data temp_l_data;
	temp_l_data.node_id = node_id;
	temp_l_data.node = node;
	temp_l_data.load_value = load_value;
	temp_l_data.load_angle = load_angle;

	// Insert the constarint data to unordered map
	// Searching for node_id
	if (l_data.find(node_id) != l_data.end())
	{
		// Node is already have constraint
		// so remove the constraint
		l_data[node_id] = temp_l_data;
		return;
	}

	// Insert the constraint to nodes
	l_data.insert({ node_id, temp_l_data });
	load_count++;
}

void mloads::delete_load(int& node_id)
{
	if (load_count != 0)
	{
		// Remove the load data to unordered map
		// Searching for node_id
		// Check there is already a constraint in the found node
		if (l_data.find(node_id) != l_data.end())
		{
			// Node is already have load
			// so remove the load
			l_data.erase(node_id);

			// adjust the constraint count
			load_count--;
		}
	}
}

void mloads::delete_all()
{
	// Delete all the resources
	load_count = 0;
	load_value_labels.delete_all();
	l_data.clear();
}

void mloads::set_buffer()
{
	// Create shader
	std::filesystem::path currentDirPath = std::filesystem::current_path();
	std::filesystem::path parentPath = currentDirPath.parent_path();
	std::filesystem::path shadersPath = parentPath / "Truss_static_analysis_cpp/src/geometry_store/shaders";

	// Load shader
	load_shader.create_shader((shadersPath.string() + "/load_vertex_shader.vert").c_str(),
		(shadersPath.string() + "/load_frag_shader.frag").c_str());
}

void mloads::update_buffer()
{
	if (load_count == 0)
	{
		// No load to paint
		return;
	}

	load_value_labels.delete_all();

	// Load Arrow Head vertices
	const unsigned int load_arrowhead_vertex_count = 3 * 9 * load_count;
	float* load_arrowhead_vertices = new float[load_arrowhead_vertex_count];

	// Load Arrow Head indices
	unsigned int load_arrowhead_indices_count = 3 * load_count;
	unsigned int* load_arrowhead_vertex_indices = new unsigned int[load_arrowhead_indices_count];

	unsigned int load_arrowhead_v_index = 0;
	unsigned int load_arrowhead_i_index = 0;

	// Load Arrow tail vertices
	const unsigned int load_arrowtail_vertex_count = 2 * 9 * load_count;
	float* load_arrowtail_vertices = new float[load_arrowtail_vertex_count];

	// Load Arrow tail indices
	unsigned int load_arrowtail_indices_count = 2 * load_count;
	unsigned int* load_arrowtail_vertex_indices = new unsigned int[load_arrowtail_indices_count];

	unsigned int load_arrowtail_v_index = 0;
	unsigned int load_arrowtail_i_index = 0;

	// Load Max
	float load_max = 0.0f;

	// Find the load maximum
	for (auto& load : l_data)
	{
		if (load_max < std::abs(load.second.load_value))
		{
			load_max = std::abs(load.second.load_value);
		}
	}

	// Set all the load labels
	for (auto& load : l_data)
	{
		// Update the load Labels
		load_data load_val = load.second;

		glm::vec3 temp_color = geom_param_ptr->geom_colors.load_color;
		std::string	temp_str = std::to_string(load_val.load_value);
		float load_angle = load_val.load_angle;


		glm::vec2 load_endpt = glm::vec2(0,
			-20.0f * (load_val.load_value / load_max) * (geom_param_ptr->node_circle_radii / geom_param_ptr->geom_scale));

		float radians = ((load_angle + 90.0f) * 3.14159365f) / 180.0f; // convert degrees to radians
		float cos_theta = cos(radians);
		float sin_theta = sin(radians);

		// Rotated point of the arrow tail
		glm::vec2 rotated_load_endpt = glm::vec2((load_endpt.x * cos_theta) + (load_endpt.y * sin_theta),
			-(load_endpt.x * sin_theta) + (load_endpt.y * cos_theta));

		nodes_store node_value = *load_val.node;
		glm::vec2 final_load_endpt = glm::vec2(node_value.node_pt.x + rotated_load_endpt.x,
			node_value.node_pt.y + rotated_load_endpt.y);

		float load_angle_rad = ((90 - load_angle) * 3.14159365f) / 180.0f;

		// std::cout << node_value.node_id << "->" << load_angle_rad << std::endl;

		load_value_labels.add_text(temp_str.c_str(), node_value.node_pt, temp_color, load_angle_rad, true);
	}

	load_value_labels.set_buffers();

	for (auto& load : l_data)
	{
		// Add the load point arrow head
		set_load_arrowhead_vertices(load_arrowhead_vertices, load_arrowhead_v_index, load.second.node,
			load.second.load_angle, load.second.load_value);

		// Add the arrow head indices
		set_load_arrowhead_indices(load_arrowhead_vertex_indices, load_arrowhead_i_index);

		// Add the load arrow tail
		set_load_arrowtail_vertices(load_arrowtail_vertices, load_arrowtail_v_index, load.second.node,
			load.second.load_angle, load.second.load_value, load_max);

		// Add the arrow tail indices
		set_load_arrowtail_indices(load_arrowtail_vertex_indices, load_arrowtail_i_index);
	}

	VertexBufferLayout load_arrowhead_layout;
	load_arrowhead_layout.AddFloat(3);  // Position
	load_arrowhead_layout.AddFloat(3);  // Center
	load_arrowhead_layout.AddFloat(3);  // Color

	unsigned int load_arrowhead_vertex_size = load_arrowhead_vertex_count * sizeof(float);

	// Create the Load Arrow Head buffers
	loadarrowhead_buffer.CreateBuffers((void*)load_arrowhead_vertices, load_arrowhead_vertex_size,
		(unsigned int*)load_arrowhead_vertex_indices, load_arrowhead_indices_count, load_arrowhead_layout);

	VertexBufferLayout load_arrowtail_layout;
	load_arrowtail_layout.AddFloat(3);  // Position
	load_arrowtail_layout.AddFloat(3);  // Center
	load_arrowtail_layout.AddFloat(3);  // Color

	unsigned int load_arrowtail_vertex_size = load_arrowtail_vertex_count * sizeof(float);

	// Create the Load Arrow Tail buffers
	loadarrowtail_buffer.CreateBuffers((void*)load_arrowtail_vertices, load_arrowtail_vertex_size,
		(unsigned int*)load_arrowtail_vertex_indices, load_arrowtail_indices_count, load_arrowtail_layout);

	// Delete the Dynamic arrays
	delete[] load_arrowhead_vertices;
	delete[] load_arrowhead_vertex_indices;
	delete[] load_arrowtail_vertices;
	delete[] load_arrowtail_vertex_indices;


}

void mloads::paint_loads()
{
	if (load_count == 0)
	{
		// No load to paint
		return;
	}


	load_shader.Bind();
	// Arrow head
	loadarrowhead_buffer.Bind();
	glDrawElements(GL_TRIANGLES, 3 * load_count, GL_UNSIGNED_INT, 0);
	loadarrowhead_buffer.UnBind();

	// Arrow tail
	loadarrowtail_buffer.Bind();
	glDrawElements(GL_LINES, 2 * load_count, GL_UNSIGNED_INT, 0);
	loadarrowtail_buffer.UnBind();

	load_shader.UnBind();
}

void mloads::paint_load_labels()
{
	// Paint the load labels
	load_value_labels.paint_text();
}

void mloads::set_load_arrowhead_vertices(float* load_arrowhead_vertices, unsigned int& load_arrowhead_v_index,
	nodes_store* node, float load_angle, float load_value)
{
	// Load arrow head vertices
	float load_arrowhead_size = (6.0f * geom_param_ptr->node_circle_radii) / geom_param_ptr->geom_scale;

	// Rotate the corner points
	glm::vec2 arrow_pt = glm::vec2(0, -(geom_param_ptr->node_circle_radii / geom_param_ptr->geom_scale)); // 0 0
	glm::vec2 arrow_hd_left = glm::vec2(-1.5f * (geom_param_ptr->node_circle_radii / geom_param_ptr->geom_scale), -5.0f * (geom_param_ptr->node_circle_radii / geom_param_ptr->geom_scale)); // -1 1
	glm::vec2 arrow_hd_right = glm::vec2(1.5f * (geom_param_ptr->node_circle_radii / geom_param_ptr->geom_scale), -5.0f * (geom_param_ptr->node_circle_radii / geom_param_ptr->geom_scale)); // 1 1

	float radians = ((load_angle + 90.0f) * 3.14159365f) / 180.0f; // convert degrees to radians
	float cos_theta = cos(radians);
	float sin_theta = sin(radians);

	// Rotated point of the corners
	glm::vec2 rotated_arrow_pt = glm::vec2((arrow_pt.x * cos_theta) + (arrow_pt.y * sin_theta),
		-(arrow_pt.x * sin_theta) + (arrow_pt.y * cos_theta));

	glm::vec2 rotated_arrow_hd_left = glm::vec2((arrow_hd_left.x * cos_theta) + (arrow_hd_left.y * sin_theta),
		-(arrow_hd_left.x * sin_theta) + (arrow_hd_left.y * cos_theta));

	glm::vec2 rotated_arrow_hd_right = glm::vec2((arrow_hd_right.x * cos_theta) + (arrow_hd_right.y * sin_theta),
		-(arrow_hd_right.x * sin_theta) + (arrow_hd_right.y * cos_theta));

	nodes_store node_value = (*node);

	// Set the load vertices Arrow point
	load_arrowhead_vertices[load_arrowhead_v_index + 0] = node_value.node_pt.x + rotated_arrow_pt.x;
	load_arrowhead_vertices[load_arrowhead_v_index + 1] = node_value.node_pt.y + rotated_arrow_pt.y;
	load_arrowhead_vertices[load_arrowhead_v_index + 2] = 0.0f;

	// Set the node center
	load_arrowhead_vertices[load_arrowhead_v_index + 3] = node_value.node_pt.x;
	load_arrowhead_vertices[load_arrowhead_v_index + 4] = node_value.node_pt.y;
	load_arrowhead_vertices[load_arrowhead_v_index + 5] = 0.0f;

	// Set the Load color
	load_arrowhead_vertices[load_arrowhead_v_index + 6] = geom_param_ptr->geom_colors.load_color.x;
	load_arrowhead_vertices[load_arrowhead_v_index + 7] = geom_param_ptr->geom_colors.load_color.y;
	load_arrowhead_vertices[load_arrowhead_v_index + 8] = geom_param_ptr->geom_colors.load_color.z;

	// Increment
	load_arrowhead_v_index = load_arrowhead_v_index + 9;

	// Set the load vertices Arrow head left pt
	load_arrowhead_vertices[load_arrowhead_v_index + 0] = node_value.node_pt.x + rotated_arrow_hd_left.x;
	load_arrowhead_vertices[load_arrowhead_v_index + 1] = node_value.node_pt.y + rotated_arrow_hd_left.y;
	load_arrowhead_vertices[load_arrowhead_v_index + 2] = 0.0f;

	// Set the node center
	load_arrowhead_vertices[load_arrowhead_v_index + 3] = node_value.node_pt.x;
	load_arrowhead_vertices[load_arrowhead_v_index + 4] = node_value.node_pt.y;
	load_arrowhead_vertices[load_arrowhead_v_index + 5] = 0.0f;

	// Set the Load color
	load_arrowhead_vertices[load_arrowhead_v_index + 6] = geom_param_ptr->geom_colors.load_color.x;
	load_arrowhead_vertices[load_arrowhead_v_index + 7] = geom_param_ptr->geom_colors.load_color.y;
	load_arrowhead_vertices[load_arrowhead_v_index + 8] = geom_param_ptr->geom_colors.load_color.z;

	// Increment
	load_arrowhead_v_index = load_arrowhead_v_index + 9;

	// Set the load vertices Arrow head right pt
	load_arrowhead_vertices[load_arrowhead_v_index + 0] = node_value.node_pt.x + rotated_arrow_hd_right.x;
	load_arrowhead_vertices[load_arrowhead_v_index + 1] = node_value.node_pt.y + rotated_arrow_hd_right.y;
	load_arrowhead_vertices[load_arrowhead_v_index + 2] = 0.0f;

	// Set the node center
	load_arrowhead_vertices[load_arrowhead_v_index + 3] = node_value.node_pt.x;
	load_arrowhead_vertices[load_arrowhead_v_index + 4] = node_value.node_pt.y;
	load_arrowhead_vertices[load_arrowhead_v_index + 5] = 0.0f;

	// Set the Load color
	load_arrowhead_vertices[load_arrowhead_v_index + 6] = geom_param_ptr->geom_colors.load_color.x;
	load_arrowhead_vertices[load_arrowhead_v_index + 7] = geom_param_ptr->geom_colors.load_color.y;
	load_arrowhead_vertices[load_arrowhead_v_index + 8] = geom_param_ptr->geom_colors.load_color.z;

	// Increment
	load_arrowhead_v_index = load_arrowhead_v_index + 9;
}

void mloads::set_load_arrowhead_indices(unsigned int* load_arrowhead_vertex_indices, unsigned int& load_arrowhead_i_index)
{
	// Load arrow head indices
	// Triangle 0,1,2
	load_arrowhead_vertex_indices[load_arrowhead_i_index + 0] = load_arrowhead_i_index + 0;
	load_arrowhead_vertex_indices[load_arrowhead_i_index + 1] = load_arrowhead_i_index + 1;
	load_arrowhead_vertex_indices[load_arrowhead_i_index + 2] = load_arrowhead_i_index + 2;

	// Increment
	load_arrowhead_i_index = load_arrowhead_i_index + 3;
}

void mloads::set_load_arrowtail_vertices(float* load_arrowtail_vertices, unsigned int& load_arrowtail_v_index, 
	nodes_store* node, float load_angle, float load_value, float load_max)
{

	int load_sign = load_value > 0 ? 1 : -1;
	// Rotate the corner points
	glm::vec2 arrow_tail_startpt = glm::vec2(0, -2.0f * load_sign * (geom_param_ptr->node_circle_radii / geom_param_ptr->geom_scale)); // 0 0
	glm::vec2 arrow_tail_endpt = glm::vec2(0, -20.0f * (load_value / load_max) * (geom_param_ptr->node_circle_radii / geom_param_ptr->geom_scale)); // -1 1

	float radians = ((load_angle + 90.0f) * 3.14159365f) / 180.0f; // convert degrees to radians
	float cos_theta = cos(radians);
	float sin_theta = sin(radians);

	// Rotated point of the arrow tail
	glm::vec2 rotated_arrow_tail_startpt = glm::vec2((arrow_tail_startpt.x * cos_theta) + (arrow_tail_startpt.y * sin_theta),
		-(arrow_tail_startpt.x * sin_theta) + (arrow_tail_startpt.y * cos_theta));

	glm::vec2 rotated_arrow_tail_endpt = glm::vec2((arrow_tail_endpt.x * cos_theta) + (arrow_tail_endpt.y * sin_theta),
		-(arrow_tail_endpt.x * sin_theta) + (arrow_tail_endpt.y * cos_theta));


	nodes_store node_value = (*node);

	// Set the load vertices Arrow tail start pt
	load_arrowtail_vertices[load_arrowtail_v_index + 0] = node_value.node_pt.x + rotated_arrow_tail_startpt.x;
	load_arrowtail_vertices[load_arrowtail_v_index + 1] = node_value.node_pt.y + rotated_arrow_tail_startpt.y;
	load_arrowtail_vertices[load_arrowtail_v_index + 2] = 0.0f;

	// Set the node center
	load_arrowtail_vertices[load_arrowtail_v_index + 3] = node_value.node_pt.x;
	load_arrowtail_vertices[load_arrowtail_v_index + 4] = node_value.node_pt.y;
	load_arrowtail_vertices[load_arrowtail_v_index + 5] = 0.0f;

	// Set the load color
	load_arrowtail_vertices[load_arrowtail_v_index + 6] = geom_param_ptr->geom_colors.load_color.x;
	load_arrowtail_vertices[load_arrowtail_v_index + 7] = geom_param_ptr->geom_colors.load_color.y;
	load_arrowtail_vertices[load_arrowtail_v_index + 8] = geom_param_ptr->geom_colors.load_color.z;

	// Increment
	load_arrowtail_v_index = load_arrowtail_v_index + 9;

	// Set the load vertices Arrow tail end pt
	load_arrowtail_vertices[load_arrowtail_v_index + 0] = node_value.node_pt.x + rotated_arrow_tail_endpt.x;
	load_arrowtail_vertices[load_arrowtail_v_index + 1] = node_value.node_pt.y + rotated_arrow_tail_endpt.y;
	load_arrowtail_vertices[load_arrowtail_v_index + 2] = 0.0f;

	// Set the node center
	load_arrowtail_vertices[load_arrowtail_v_index + 3] = node_value.node_pt.x;
	load_arrowtail_vertices[load_arrowtail_v_index + 4] = node_value.node_pt.y;
	load_arrowtail_vertices[load_arrowtail_v_index + 5] = 0.0f;

	// Set the load color
	load_arrowtail_vertices[load_arrowtail_v_index + 6] = geom_param_ptr->geom_colors.load_color.x;
	load_arrowtail_vertices[load_arrowtail_v_index + 7] = geom_param_ptr->geom_colors.load_color.y;
	load_arrowtail_vertices[load_arrowtail_v_index + 8] = geom_param_ptr->geom_colors.load_color.z;

	// Increment
	load_arrowtail_v_index = load_arrowtail_v_index + 9;
}

void mloads::set_load_arrowtail_indices(unsigned int* load_arrowtail_vertex_indices, unsigned int& load_arrowtail_i_index)
{
	// Load arrow tail indices
	// Line 0,1
	load_arrowtail_vertex_indices[load_arrowtail_i_index + 0] = load_arrowtail_i_index + 0;
	load_arrowtail_vertex_indices[load_arrowtail_i_index + 1] = load_arrowtail_i_index + 1;

	// Increment
	load_arrowtail_i_index = load_arrowtail_i_index + 2;
}

void mloads::update_geometry_matrices(bool is_modelmatrix, bool is_pantranslation, bool is_zoomtranslation, bool set_transparency)
{
	// Set Transparency
	if (set_transparency == true)
	{
		load_shader.setUniform("transparency", geom_param_ptr->geom_transparency);
	}

	// Model Matrix
	if (is_modelmatrix == true)
	{
		load_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
	}

	// Pan Translation
	if (is_pantranslation == true)
	{
		load_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
	}

	// Zoom Translation
	if (is_zoomtranslation == true)
	{
		load_shader.setUniform("zoomscale", geom_param_ptr->zoom_scale);
	}
}
