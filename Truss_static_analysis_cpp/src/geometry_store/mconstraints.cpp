#include "mconstraints.h"

mconstraints::mconstraints()
	:constraint_count(0)
{
	// Constructor
}

mconstraints::~mconstraints()
{
	// Destructor
}

void mconstraints::init(geom_parameters* geom_param_ptr)
{
	this->geom_param_ptr = geom_param_ptr;

	// Delete all the constraints
	constraint_count = 0;
	c_data.clear();
}

void mconstraints::add_constraint(int& node_id, nodes_store* node, int& constraint_type, float& constraint_angle)
{
	// Add constraints
	constraint_data temp_c_data;
	temp_c_data.node_id = node_id;
	temp_c_data.node = node;
	temp_c_data.constraint_type = constraint_type;
	temp_c_data.constraint_angle = constraint_angle;

	// Insert the constarint data to unordered map
	// Searching for node_id
	if (c_data.find(node_id) != c_data.end())
	{
		// Node is already have constraint
		// so remove the constraint
		c_data[node_id] = temp_c_data;
		return;
	}

	// Insert the constraint to nodes
	c_data.insert({ node_id, temp_c_data });
	constraint_count++;
}

void mconstraints::delete_constraint(int& node_id)
{
	if (constraint_count != 0)
	{
		// Remove the constarint data to unordered map
		// Searching for node_id
		// Check there is already a constraint in the found node
		if (c_data.find(node_id) != c_data.end())
		{
			// Node is already have constraint
			// so remove the constraint
			c_data.erase(node_id);

			// adjust the constraint count
			constraint_count--;
		}
	}
}

void mconstraints::delete_all()
{
	// Delete all the resources
	constraint_count = 0;
	c_data.clear();
}

void mconstraints::set_buffer()
{
	// Create shader
	std::filesystem::path currentDirPath = std::filesystem::current_path();
	std::filesystem::path parentPath = currentDirPath.parent_path();
	std::filesystem::path shadersPath = parentPath / "Truss_static_analysis_cpp/src/geometry_store/shaders";

	// Load shader
	constraint_shader.create_shader((shadersPath.string() + "/constraint_vertex_shader.vert").c_str(),
		(shadersPath.string() + "/constraint_frag_shader.frag").c_str());

	// Load textures
	constraint_texture_pin.LoadTexture((shadersPath.string() + "/pic_pin_support.png").c_str());
	constraint_texture_roller.LoadTexture((shadersPath.string() + "/pic_roller_support.png").c_str());

	// Set texture uniform variables
	constraint_shader.setUniform("u_Textures[0]", 0);
	constraint_shader.setUniform("u_Textures[1]", 1);
}

void mconstraints::update_buffer()
{
	if (constraint_count == 0)
	{
		// No constraint to paint
		return;
	}

	const unsigned int constraint_vertex_count = 4 * 12 * constraint_count;
	float* constraint_vertices = new float[constraint_vertex_count];

	unsigned int constraint_indices_count = 6 * constraint_count;
	unsigned int* constraint_vertex_indices = new unsigned int[constraint_indices_count];

	unsigned int constraint_v_index = 0;
	unsigned int constraint_i_index = 0;

	for (auto& constraint : c_data)
	{
		// Add the constraint point
		set_constraint_vertices(constraint_vertices, constraint_v_index, constraint.second.node,
			constraint.second.constraint_angle, unsigned int(constraint.second.constraint_type));

		// Add the indices
		set_constraint_indices(constraint_vertex_indices, constraint_i_index);
	}

	VertexBufferLayout constraint_layout;
	constraint_layout.AddFloat(3);  // Position
	constraint_layout.AddFloat(3);  // Center
	constraint_layout.AddFloat(3);  // Color
	constraint_layout.AddFloat(2);  // Texture co-ordinate
	constraint_layout.AddFloat(1);  // Texture  type

	unsigned int constraint_vertex_size = constraint_vertex_count * sizeof(float);

	// Create the Constraint buffers
	constraint_buffer.CreateBuffers((void*)constraint_vertices, constraint_vertex_size,
		(unsigned int*)constraint_vertex_indices, constraint_indices_count, constraint_layout);

	// Delete the Dynamic arrays
	delete[] constraint_vertices;
	delete[] constraint_vertex_indices;
}


void mconstraints::paint_constraints()
{
	if (constraint_count == 0)
	{
		// No constraint to paint
		return;
	}

	constraint_shader.Bind();
	constraint_buffer.Bind();

	// Activate textures (pin and roller support)
	constraint_texture_pin.Bind();
	constraint_texture_roller.Bind(1);

	glDrawElements(GL_TRIANGLES, 6 * constraint_count, GL_UNSIGNED_INT, 0);

	constraint_texture_pin.UnBind();
	constraint_texture_roller.UnBind();

	constraint_buffer.UnBind();
	constraint_shader.UnBind();
}


void mconstraints::set_constraint_vertices(float* constraint_vertices, unsigned int& constraint_v_index, nodes_store* node, float constraint_angle, unsigned int constraint_type)
{
	// Set the Constraint vertices
	float constraint_size = (6.0f * geom_param_ptr->node_circle_radii) / geom_param_ptr->geom_scale;

	// Rotate the corner points
	glm::vec2 bot_left = glm::vec2(-constraint_size, -constraint_size); // 0 0
	glm::vec2 bot_right = glm::vec2(constraint_size, -constraint_size); // 1 0
	glm::vec2 top_right = glm::vec2(constraint_size, constraint_size); // 1 1
	glm::vec2 top_left = glm::vec2(-constraint_size, constraint_size); // 0 1


	float radians = ((constraint_angle + 90.0f) * 3.14159365f) / 180.0f; // convert degrees to radians
	float cos_theta = cos(radians);
	float sin_theta = sin(radians);

	// Rotated point of the corners
	glm::vec2 rotated_pt_bot_left = glm::vec2((bot_left.x * cos_theta) + (bot_left.y * sin_theta),
		-(bot_left.x * sin_theta) + (bot_left.y * cos_theta));

	glm::vec2 rotated_pt_bot_right = glm::vec2((bot_right.x * cos_theta) + (bot_right.y * sin_theta),
		-(bot_right.x * sin_theta) + (bot_right.y * cos_theta));

	glm::vec2 rotated_pt_top_right = glm::vec2((top_right.x * cos_theta) + (top_right.y * sin_theta),
		-(top_right.x * sin_theta) + (top_right.y * cos_theta));

	glm::vec2 rotated_pt_top_left = glm::vec2((top_left.x * cos_theta) + (top_left.y * sin_theta),
		-(top_left.x * sin_theta) + (top_left.y * cos_theta));


	nodes_store node_value = (*node);

	// Set the Constraint vertices Corner 1
	constraint_vertices[constraint_v_index + 0] = node_value.node_pt.x + rotated_pt_bot_left.x;
	constraint_vertices[constraint_v_index + 1] = node_value.node_pt.y + rotated_pt_bot_left.y;
	constraint_vertices[constraint_v_index + 2] = 0.0f;

	// Set the node center
	constraint_vertices[constraint_v_index + 3] = node_value.node_pt.x;
	constraint_vertices[constraint_v_index + 4] = node_value.node_pt.y;
	constraint_vertices[constraint_v_index + 5] = 0.0f;

	// Set the node color
	constraint_vertices[constraint_v_index + 6] = geom_param_ptr->geom_colors.constraint_color.x;
	constraint_vertices[constraint_v_index + 7] = geom_param_ptr->geom_colors.constraint_color.y;
	constraint_vertices[constraint_v_index + 8] = geom_param_ptr->geom_colors.constraint_color.z;

	// Set the Texture co-ordinates
	constraint_vertices[constraint_v_index + 9] = 0.0f;
	constraint_vertices[constraint_v_index + 10] = 0.0f;

	// Texture type
	constraint_vertices[constraint_v_index + 11] = constraint_type;

	// Increment
	constraint_v_index = constraint_v_index + 12;

	// Set the Constraint vertices Corner 2
	constraint_vertices[constraint_v_index + 0] = node_value.node_pt.x + rotated_pt_bot_right.x;
	constraint_vertices[constraint_v_index + 1] = node_value.node_pt.y + rotated_pt_bot_right.y;
	constraint_vertices[constraint_v_index + 2] = 0.0f;

	// Set the node center
	constraint_vertices[constraint_v_index + 3] = node_value.node_pt.x;
	constraint_vertices[constraint_v_index + 4] = node_value.node_pt.y;
	constraint_vertices[constraint_v_index + 5] = 0.0f;

	// Set the node color
	constraint_vertices[constraint_v_index + 6] = geom_param_ptr->geom_colors.constraint_color.x;
	constraint_vertices[constraint_v_index + 7] = geom_param_ptr->geom_colors.constraint_color.y;
	constraint_vertices[constraint_v_index + 8] = geom_param_ptr->geom_colors.constraint_color.z;

	// Set the Texture co-ordinates
	constraint_vertices[constraint_v_index + 9] = 1.0f;
	constraint_vertices[constraint_v_index + 10] = 0.0f;

	// Texture type
	constraint_vertices[constraint_v_index + 11] = constraint_type;

	// Increment
	constraint_v_index = constraint_v_index + 12;

	// Set the Constraint vertices Corner 3
	constraint_vertices[constraint_v_index + 0] = node_value.node_pt.x + rotated_pt_top_right.x;
	constraint_vertices[constraint_v_index + 1] = node_value.node_pt.y + rotated_pt_top_right.y;
	constraint_vertices[constraint_v_index + 2] = 0.0f;

	// Set the node center
	constraint_vertices[constraint_v_index + 3] = node_value.node_pt.x;
	constraint_vertices[constraint_v_index + 4] = node_value.node_pt.y;
	constraint_vertices[constraint_v_index + 5] = 0.0f;

	// Set the node color
	constraint_vertices[constraint_v_index + 6] = geom_param_ptr->geom_colors.constraint_color.x;
	constraint_vertices[constraint_v_index + 7] = geom_param_ptr->geom_colors.constraint_color.y;
	constraint_vertices[constraint_v_index + 8] = geom_param_ptr->geom_colors.constraint_color.z;

	// Set the Texture co-ordinates
	constraint_vertices[constraint_v_index + 9] = 1.0f;
	constraint_vertices[constraint_v_index + 10] = 1.0f;

	// Texture type
	constraint_vertices[constraint_v_index + 11] = constraint_type;

	// Increment
	constraint_v_index = constraint_v_index + 12;

	// Set the Constraint vertices Corner 4
	constraint_vertices[constraint_v_index + 0] = node_value.node_pt.x + rotated_pt_top_left.x;
	constraint_vertices[constraint_v_index + 1] = node_value.node_pt.y + rotated_pt_top_left.y;
	constraint_vertices[constraint_v_index + 2] = 0.0f;

	// Set the node center
	constraint_vertices[constraint_v_index + 3] = node_value.node_pt.x;
	constraint_vertices[constraint_v_index + 4] = node_value.node_pt.y;
	constraint_vertices[constraint_v_index + 5] = 0.0f;

	// Set the node color
	constraint_vertices[constraint_v_index + 6] = geom_param_ptr->geom_colors.constraint_color.x;
	constraint_vertices[constraint_v_index + 7] = geom_param_ptr->geom_colors.constraint_color.y;
	constraint_vertices[constraint_v_index + 8] = geom_param_ptr->geom_colors.constraint_color.z;

	// Set the Texture co-ordinates
	constraint_vertices[constraint_v_index + 9] = 0.0f;
	constraint_vertices[constraint_v_index + 10] = 1.0f;

	// Texture type
	constraint_vertices[constraint_v_index + 11] = constraint_type;

	// Increment
	constraint_v_index = constraint_v_index + 12;
}

void mconstraints::set_constraint_indices(unsigned int* constraint_vertex_indices, unsigned int& constraint_i_index)
{
	// Set the node indices
	unsigned int t_id = ((constraint_i_index / 6) * 4);
	// Triangle 0,1,2
	constraint_vertex_indices[constraint_i_index + 0] = t_id + 0;
	constraint_vertex_indices[constraint_i_index + 1] = t_id + 1;
	constraint_vertex_indices[constraint_i_index + 2] = t_id + 2;

	// Triangle 2,3,0
	constraint_vertex_indices[constraint_i_index + 3] = t_id + 2;
	constraint_vertex_indices[constraint_i_index + 4] = t_id + 3;
	constraint_vertex_indices[constraint_i_index + 5] = t_id + 0;

	// Increment
	constraint_i_index = constraint_i_index + 6;
}



void mconstraints::update_geometry_matrices(bool is_modelmatrix, bool is_pantranslation, bool is_zoomtranslation, bool set_transparency)
{
	// Set Transparency
	if (set_transparency == true)
	{
		constraint_shader.setUniform("transparency", geom_param_ptr->geom_transparency);
	}

	// Model Matrix
	if (is_modelmatrix == true)
	{
		constraint_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
	}

	// Pan Translation
	if (is_pantranslation == true)
	{
		constraint_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
	}

	// Zoom Translation
	if (is_zoomtranslation == true)
	{
		constraint_shader.setUniform("zoomscale", geom_param_ptr->zoom_scale);
	}
}