#include "geom_store.h"

geom_store::geom_store()
	: is_geometry_set(false),
	is_geometry_loaded(false),
	node_count(0), line_count(0),
	min_b(glm::vec3(0)),
	max_b(glm::vec3(0)),
	geom_bound(glm::vec3(0)), modelMatrix(glm::mat4(0)), geom_scale(1.0f), panTranslation(glm::mat4(0)),
	center(glm::vec3(0)), window_width(0), window_height(0), zoom_scale(1.0f)
{
	// Empty constructor
	// ,line_buffer(), node_buffer(), node_texture(), node_sh(), model_sh()
}

void geom_store::write_rawdata(std::ofstream& file)
{
	// Write all the nodes
	for (auto& node : nodeMap)
	{
		// Print the node details
		nodes_store nd_val = node.second;

		file << "node, " 
			<< nd_val.node_id << ", "
			<< nd_val.node_pt.x << ", "
			<<nd_val.node_pt.y<< std::endl;
	}

	// Write all the lines
	for (auto& line : lineMap)
	{
		// Print the line details
		lines_store ln_val = line.second;

		file << "line, "
			<< ln_val.line_id << ", "
			<< ln_val.startNode.node_id << ", "
			<< ln_val.endNode.node_id <<", "
			<< ln_val.material_id <<std::endl;
	}

	// Write all the constraints
	for (auto& cnst : constraintMap.c_data)
	{
		// Print the constraint details
		constraint_data cn_val = cnst.second;

		file << "cnst, "
			<< cn_val.node_id << ", "
			<< cn_val.constraint_type << ", "
			<< cn_val.constraint_angle << std::endl;
	}

	// Write all the loads
	for (auto& ld : loadMap.l_data)
	{
		// Print the load details
		load_data ld_val = ld.second;

		file << "load, "
			<< ld_val.node_id << ", "
			<< ld_val.load_value << ", "
			<< ld_val.load_angle << std::endl;
	}

	// Write all the material property
	for (auto& mat : mat_window->material_list)
	{
		file << "mtrl, " << ", "
			<< mat.material_id << ", "
			<< mat.material_name << ", "
			<< mat.youngs_mod << ", "
			<< mat.mat_density << ", "
			<< mat.cs_area << std::endl;
	}
}

void geom_store::read_rawdata(std::ifstream& input_file)
{
	// Read the Raw Data
	// Read the entire file into a string
	std::string file_contents((std::istreambuf_iterator<char>(input_file)),
		std::istreambuf_iterator<char>());

	// Split the string into lines
	std::istringstream iss(file_contents);
	std::string line;
	std::vector<std::string> lines;
	while (std::getline(iss, line))
	{
		lines.push_back(line);
	}

	int j = 0, i = 0;

	// Create an unordered_map to store nodes with ID as key
	std::unordered_map<int, nodes_store> nodeMap;
	// Create an unordered_map to store lines with ID as key
	std::unordered_map<int, lines_store> lineMap;
	// Constraint data store
	mconstraints constraintMap;
	// Load data store
	mloads loadMap;
	// Material data list
	std::vector<material_data> mat_data;

	// Process the lines
	while (j < lines.size())
	{
		std::string line = lines[j];
		std::string type = line.substr(0, 4);  // Extract the first 4 characters of the line

		// Split the line into comma-separated fields
		std::istringstream iss(line);
		std::string field;
		std::vector<std::string> fields;
		while (std::getline(iss, field, ','))
		{
			fields.push_back(field);
		}

		if (type == "node")
		{
			// Read the nodes
			int node_id = std::stoi(fields[1]); // node ID
			float x = std::stof(fields[2]); // Node coordinate x
			float y = std::stof(fields[3]); // Node coordinate y

			// Add to node Map
			nodes_store node;
			node.add_node(node_id, glm::vec3(x, y, 0.0f));
			nodeMap[node_id] = node;
		}
		else if (type == "line")
		{
			int line_id = std::stoi(fields[1]); // line ID
			int start_node_id = std::stoi(fields[2]); // line id start node
			int end_node_id = std::stoi(fields[3]); // line id end node
			int material_id = std::stoi(fields[4]); // materail ID of the line

			// Add to line Map (Note that Nodes needed to be added before the start of line addition !!!!)
			lines_store line;
			line.add_line(line_id, nodeMap[start_node_id], nodeMap[end_node_id], material_id);
			lineMap[line_id] = line;
		}
		else if (type == "cnst")
		{
			int cnst_nd_id = std::stoi(fields[1]); // constraint node ID
			int cnst_type = std::stoi(fields[2]); // constraint type 
			float cnst_angle = std::stof(fields[3]); // constraint angle

			// Add to constraint map
			constraintMap.add_constraint(cnst_nd_id, &nodeMap[cnst_nd_id], cnst_type, cnst_angle);
		}
		else if (type == "load")
		{
			int load_nd_id = std::stoi(fields[1]); // load node ID
			float load_val = std::stof(fields[2]); // load value
			float load_angle = std::stof(fields[3]); // load angle

			// Add to load map
			loadMap.add_load(load_nd_id, &nodeMap[load_nd_id], load_val, load_angle);
		}
		else if (type == "load")
		{
			int load_nd_id = std::stoi(fields[1]); // load node ID
			float load_val = std::stof(fields[2]); // load value
			float load_angle = std::stof(fields[3]); // load angle

			// Add to load map
			loadMap.add_load(load_nd_id, &nodeMap[load_nd_id], load_val, load_angle);
		}
		else if (type == "mtrl")
		{
			// Material data
			material_data inpt_material;
			inpt_material.material_id = std::stoi(fields[1]); // Get the material id
			inpt_material.material_name = fields[2]; // Get the material name
			inpt_material.mat_density = std::stod(fields[3]); // Get the material youngs modulus
			inpt_material.youngs_mod = std::stod(fields[4]); // Get the material density
			inpt_material.cs_area = std::stod(fields[5]); // Get the material cross section area

			// Add to materail list
			mat_data.push_back(inpt_material);
		}

		// Iterate line
		j++;
	}


	// Data loaded create the geometry

	if (nodeMap.size() < 1 || lineMap.size() < 1)
	{
		// No elements added
		return;
	}

	//Add the materail list
	mat_window->material_list = mat_data;
	
	// Re-instantitize geom_store object using the nodeMap and lineMap
	deleteResources();
	create_geometry(nodeMap, lineMap,constraintMap,loadMap);
}

void geom_store::read_varai2d(std::ifstream& input_file)
{
	// Read the varai2D
	// Read the entire file into a string
	std::string file_contents((std::istreambuf_iterator<char>(input_file)),
		std::istreambuf_iterator<char>());

	// Split the string into lines
	std::istringstream iss(file_contents);
	std::string line;
	std::vector<std::string> lines;
	while (std::getline(iss, line))
	{
		lines.push_back(line);
	}

	int j = 0, i = 0;


	// Create an unordered_map to store nodes with ID as key
	std::unordered_map<int, nodes_store> nodeMap;
	// Create an unordered_map to store lines with ID as key
	std::unordered_map<int, lines_store> lineMap;

	// Process the lines
	while (j < lines.size())
	{
		std::cout << "Line: " << lines[j] << std::endl;
		// Check for the start of nodes input
		if (lines[j].find("[+] End Points") != std::string::npos)
		{
			int num_nodes;
			// Read the number of nodes
			std::stringstream ss(lines[j]);
			std::string token;
			std::getline(ss, token, ','); // Get the string "[+] End Points"
			std::getline(ss, token, ','); // Get the number of nodes as a string
			num_nodes = std::stoi(token) + j; // Convert the string to an integer

			// Read and store the nodes
			for (i = j; i < num_nodes; i++)
			{
				int node_id;
				float x, y;

				std::stringstream ss(lines[i + 1]);
				std::string token;

				std::getline(ss, token, ','); // read the node ID
				node_id = std::stoi(token);

				std::getline(ss, token, ','); // read the x-coordinate
				x = std::stof(token);

				std::getline(ss, token, ','); // read the y-coordinate
				y = std::stof(token);

				// Create nodes_store object and store in nodeMap
				nodes_store node;
				node.add_node(node_id, glm::vec3(x, y, 0.0f));
				nodeMap[node_id] = node;
				j++;
			}
		}
		// Check for the start of lines input
		else if (lines[j].find("[+] Lines") != std::string::npos) {
			int num_lines;
			// Read the number of nodes
			std::stringstream ss(lines[j]);
			std::string token;
			std::getline(ss, token, ','); // Get the string "[+] Lines"
			std::getline(ss, token, ','); // Get the number of nodes as a string
			num_lines = std::stoi(token) + j; // Convert the string to an integer

			// Read and store the lines
			for (i = j; i < num_lines; i++)
			{
				int line_id, start_node_id, end_node_id;
				std::stringstream ss(lines[i + 1]);
				std::string token;

				std::getline(ss, token, ','); // read the line ID
				line_id = std::stoi(token);

				std::getline(ss, token, ','); // read the start node ID
				start_node_id = std::stoi(token);

				std::getline(ss, token, ','); // read the end node ID
				end_node_id = std::stoi(token);

				// Create lines_store object using references to startNode and endNode
				lines_store line;
				line.add_line(line_id, nodeMap[start_node_id], nodeMap[end_node_id],0);
				lineMap[line_id] = line;
				j++;
			}
		}

		// iterate line
		j++;
	}

	if (nodeMap.size() < 1 || lineMap.size() < 1)
	{
		// No elements added
		return;
	}

	// Add a default material to the materail window
		//default_material.material_name = "Default material";
	//default_material.mat_density = 7.83 * std::pow(10, -9); // tons/mm3
	//default_material.youngs_mod = 2.07 * std::pow(10, 5); // MPa
	//default_material.cs_area = 6014; // mm2


	material_data inpt_material;
	inpt_material.material_id = 0; // Get the material id
	inpt_material.material_name = "Default material"; //Default material name
	inpt_material.mat_density = 7.83 * std::pow(10, -9); // tons/mm3
	inpt_material.youngs_mod = 2.07 * std::pow(10, 5); //  MPa
	inpt_material.cs_area = 6014; // mm2

	// Add to materail list
	mat_window->material_list.clear();
	mat_window->material_list.push_back(inpt_material);


	mconstraints constraintMap;
	mloads loadMap;
	// Re-instantitize geom_store object using the nodeMap and lineMap
	deleteResources();
	create_geometry(nodeMap, lineMap, constraintMap, loadMap);
}


void geom_store::create_geometry(std::unordered_map<int, nodes_store>& nodeMap,
	std::unordered_map<int, lines_store>& lineMap,
	mconstraints& constraintMap,
	mloads& loadMap)
{
	// Constructor
	this->nodeMap = nodeMap;
	this->lineMap = lineMap;

	this->constraintMap = constraintMap;
	this->loadMap = loadMap;

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

	// Create the geometry labels (Node ID, Node Coord, Line ID, Line Length)
	create_geometry_labels();
	update_constraint();
	update_load();
}

geom_store::~geom_store()
{
	// Destructor
	deleteResources();
}

void geom_store::add_window_ptr(options_window* op_window, material_window* mat_window)
{
	// Add the pointer of the windows (material and option window)
	this->op_window = op_window;
	this->mat_window = mat_window;
}

void geom_store::deleteResources()
{
	is_geometry_loaded = false;
	is_geometry_set = false;
	// Call the destructor for each nodes_store object in the nodeMap
	for (auto& node : nodeMap)
	{
		node.second.~nodes_store();
	}
	// Call the destructor for each lines_store object in the lineMap
	for (auto& line : lineMap)
	{
		line.second.~lines_store();
	}

	// Clear the nodeMap and lineMap
	nodeMap.clear();
	lineMap.clear();

	// Clear the constraints and loads from previous model
	constraintMap.delete_all();
	loadMap.delete_all();
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

	// Define the node vertices of the model (4 vertex (to form a triangle) for a node (3 position, 3 color, 3 center & 2 texture coordinate) 
	const unsigned int node_vertex_count = 4 * 11 * node_count;
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
		line_indices[line_i_index + 0] = node_id_map[line.second.startNode.node_id];
		line_indices[line_i_index + 1] = node_id_map[line.second.endNode.node_id];

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
	node_layout.AddFloat(3);  // Node center
	node_layout.AddFloat(3);  // Color
	node_layout.AddFloat(2);  // Texture co-ordinate

	// Create the Node buffers
	node_buffer.CreateBuffers((void*)node_vertices, node_vertex_size, (unsigned int*)node_vertex_indices, node_indices_count, node_layout);

	// Create shader
	std::filesystem::path currentDirPath = std::filesystem::current_path();
	std::filesystem::path parentPath = currentDirPath.parent_path();
	std::filesystem::path shadersPath = parentPath / "Truss_static_analysis_cpp/src/geometry_store/shaders";
	std::string parentString = shadersPath.string();
	std::cout << "Parent path: " << parentString << std::endl;

	// Model shader
	line_shader.create_shader((shadersPath.string() + "/geom_vertex_shader.vert").c_str(),
		(shadersPath.string() + "/geom_frag_shader.frag").c_str());

	// Node shader
	node_shader.create_shader((shadersPath.string() + "/node_vertex_shader.vert").c_str(),
		(shadersPath.string() + "/node_frag_shader.frag").c_str());

	node_texture.LoadTexture((shadersPath.string() + "/pic_3d_circle_paint.png").c_str());
	node_shader.setUniform("u_Texture", 0);

	// Constraint shader
	constraint_shader.create_shader((shadersPath.string() + "/constraint_vertex_shader.vert").c_str(),
		(shadersPath.string() + "/constraint_frag_shader.frag").c_str());

	// Load textures
	constraint_texture_pin.LoadTexture((shadersPath.string() + "/pic_pin_support.png").c_str());
	constraint_texture_roller.LoadTexture((shadersPath.string() + "/pic_roller_support.png").c_str());

	// Set texture uniform variables
	constraint_shader.setUniform("u_Textures[0]", 0);
	constraint_shader.setUniform("u_Textures[1]", 1);

	// Load shader
	load_shader.create_shader((shadersPath.string() + "/load_vertex_shader.vert").c_str(),
		(shadersPath.string() + "/load_frag_shader.frag").c_str());

	// Text shader
	text_shader.create_shader((shadersPath.string() + "/text_vert_shader.vert").c_str(),
		(shadersPath.string() + "/text_frag_shader.frag").c_str());

	// Set texture uniform variables
	text_shader.setUniform("u_Texture", 0);

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
	float node_size = node_circle_radii / geom_scale;

	// Set the node vertices Corner 1
	node_vertices[node_v_index + 0] = node.node_pt.x - node_size;
	node_vertices[node_v_index + 1] = node.node_pt.y - node_size;
	node_vertices[node_v_index + 2] = 0.0f;

	// node center
	node_vertices[node_v_index + 3] = node.node_pt.x;
	node_vertices[node_v_index + 4] = node.node_pt.y;
	node_vertices[node_v_index + 5] = 0.0f;

	// Set the node color
	node_vertices[node_v_index + 6] = node.default_color.x;
	node_vertices[node_v_index + 7] = node.default_color.y;
	node_vertices[node_v_index + 8] = node.default_color.z;

	// Set the Texture co-ordinates
	node_vertices[node_v_index + 9] = 0.0f;
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
	node_vertices[node_v_index + 6] = node.default_color.x;
	node_vertices[node_v_index + 7] = node.default_color.y;
	node_vertices[node_v_index + 8] = node.default_color.z;

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
	node_vertices[node_v_index + 6] = node.default_color.x;
	node_vertices[node_v_index + 7] = node.default_color.y;
	node_vertices[node_v_index + 8] = node.default_color.z;

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
	node_vertices[node_v_index + 6] = node.default_color.x;
	node_vertices[node_v_index + 7] = node.default_color.y;
	node_vertices[node_v_index + 8] = node.default_color.z;

	// Set the Texture co-ordinates
	node_vertices[node_v_index + 9] = 0.0f;
	node_vertices[node_v_index + 10] = 1.0f;

	// Increment
	node_v_index = node_v_index + 11;
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
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Paint the Lines
	line_shader.Bind();
	line_buffer.Bind();
	glDrawElements(GL_LINES, 2 * line_count, GL_UNSIGNED_INT, 0);
	line_buffer.UnBind();
	line_shader.UnBind();

	// Paint the Nodes
	node_shader.Bind();
	node_buffer.Bind();
	node_texture.Bind();
	glDrawElements(GL_TRIANGLES, 6 * node_count, GL_UNSIGNED_INT, 0);
	node_texture.UnBind();
	node_buffer.UnBind();
	node_shader.UnBind();

	// Paint the Constraints
	if (constraintMap.constraint_count != 0)
	{
		constraint_shader.Bind();
		constraint_buffer.Bind();

		// Activate textures (pin and roller support)
		constraint_texture_pin.Bind();
		constraint_texture_roller.Bind(1);

		glDrawElements(GL_TRIANGLES, 6 * constraintMap.constraint_count, GL_UNSIGNED_INT, 0);

		constraint_texture_pin.UnBind();
		constraint_texture_roller.UnBind();

		constraint_buffer.UnBind();
		constraint_shader.UnBind();
	}

	// Paint the Loads
	if (loadMap.load_count != 0)
	{
		load_shader.Bind();
		// Arrow head
		loadarrowhead_buffer.Bind();
		glDrawElements(GL_TRIANGLES, 3 * loadMap.load_count, GL_UNSIGNED_INT, 0);
		loadarrowhead_buffer.UnBind();

		// Arrow tail
		loadarrowtail_buffer.Bind();
		glDrawElements(GL_LINES, 2 * loadMap.load_count, GL_UNSIGNED_INT, 0);
		loadarrowtail_buffer.UnBind();

		load_shader.UnBind();
	}

	// Paint the labels
	text_shader.Bind();

	if (op_window->is_show_nodenumber == true)
	{
		// Show node ids
		node_id_labels.paint_text();
	}

	if (op_window->is_show_nodecoord == true)
	{
		// Show node coordinate
		node_coord_labels.paint_text();
	}

	if (op_window->is_show_linenumber == true)
	{
		// Show line id
		line_id_labels.paint_text();
	}

	if (op_window->is_show_linelength == true)
	{
		// Show line length
		line_length_labels.paint_text();
	}

	if (op_window->is_show_loadvalue == true && loadMap.load_count != 0)
	{
		// Show the load value
		load_value_labels.paint_text();
	}

	text_shader.UnBind();
}

void geom_store::set_model_matrix()
{
	// Set the model matrix for the model shader
	// Find the scale of the model (with 0.9 being the maximum used)
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

	modelMatrix = g_transl * glm::scale(glm::mat4(1.0f), glm::vec3(geom_scale));

	line_shader.setUniform("modelMatrix", modelMatrix, false);
	node_shader.setUniform("modelMatrix", modelMatrix, false);
	constraint_shader.setUniform("modelMatrix", modelMatrix, false);
	load_shader.setUniform("modelMatrix", modelMatrix, false);
	text_shader.setUniform("modelMatrix", modelMatrix, false);
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

	line_shader.setUniform("rotationMatrix", rotationMatrix, false);
	node_shader.setUniform("rotationMatrix", rotationMatrix, false);
	constraint_shader.setUniform("rotationMatrix", rotationMatrix, false);
	load_shader.setUniform("rotationMatrix", rotationMatrix, false);
	text_shader.setUniform("rotationMatrix", rotationMatrix, false);

	// Set the pan translation matrix
	panTranslation = glm::mat4(1.0f);

	line_shader.setUniform("panTranslation", panTranslation, false);
	node_shader.setUniform("panTranslation", panTranslation, false);
	constraint_shader.setUniform("panTranslation", panTranslation, false);
	load_shader.setUniform("panTranslation", panTranslation, false);
	text_shader.setUniform("panTranslation", panTranslation, false);

	// Set the zoom matrix
	zoom_scale = 1.0f;

	line_shader.setUniform("zoomscale", zoom_scale);
	node_shader.setUniform("zoomscale", zoom_scale);
	constraint_shader.setUniform("zoomscale", zoom_scale);
	load_shader.setUniform("zoomscale", zoom_scale);
	text_shader.setUniform("zoomscale", zoom_scale);
}

void geom_store::pan_geometry(glm::vec2& transl)
{
	// Pan the geometry
	panTranslation = glm::mat4(1.0f);

	panTranslation[0][3] = -1.0f * transl.x;
	panTranslation[1][3] = transl.y;

	line_shader.setUniform("panTranslation", panTranslation, false);
	node_shader.setUniform("panTranslation", panTranslation, false);
	constraint_shader.setUniform("panTranslation", panTranslation, false);
	load_shader.setUniform("panTranslation", panTranslation, false);
	text_shader.setUniform("panTranslation", panTranslation, false);
}

void geom_store::zoom_geometry(float& z_scale)
{
	zoom_scale = z_scale;

	// Zoom the geometry
	line_shader.setUniform("zoomscale", zoom_scale);
	node_shader.setUniform("zoomscale", zoom_scale);
	constraint_shader.setUniform("zoomscale", zoom_scale);
	load_shader.setUniform("zoomscale", zoom_scale);
	text_shader.setUniform("zoomscale", zoom_scale);
}

void geom_store::set_nodal_loads(glm::vec2& loc, float& load_value, float& load_angle, bool is_add)
{
	int node_hit_id = -1;
	// Set the nodal loads
	if (is_geometry_set == true)
	{
		if (is_add == true)
		{
			// Add load
			node_hit_id = is_node_hit(loc);
			if (node_hit_id != -1)
			{
				loadMap.add_load(node_hit_id, &nodeMap[node_hit_id], load_value, load_angle);
				// Node hit == True
				// std::cout << "Node Hit: " << node_hit_id << std::endl;
			}
		}
		else
		{
			// Remove load
			node_hit_id = is_node_hit(loc);
			if (node_hit_id != -1)
			{
				loadMap.delete_load(node_hit_id);
				// Node hit == True
				// std::cout << "Node Hit: " << node_hit_id << std::endl;
			}
		}
	}

	// Update the load vertices
	if (node_hit_id != -1)
	{
		update_load();
	}
}

void geom_store::set_nodal_constraints(glm::vec2& loc, int& constraint_type, float& constraint_angle, bool is_add)
{
	int node_hit_id = -1;
	// Set the nodal constraints
	if (is_geometry_set == true)
	{
		if (is_add == true)
		{
			// Add constraints
			node_hit_id = is_node_hit(loc);
			if (node_hit_id != -1)
			{
				constraintMap.add_constraint(node_hit_id, &nodeMap[node_hit_id], constraint_type, constraint_angle);
				// Node hit == True
				// std::cout << "Node Hit: " << node_hit_id << std::endl;
			}
		}
		else
		{
			// Remove constraints
			node_hit_id = is_node_hit(loc);
			if (node_hit_id != -1)
			{
				constraintMap.delete_constraint(node_hit_id);
				// Node hit == True
				// std::cout << "Node Hit: " << node_hit_id << std::endl;
			}
		}
	}

	// Update the constraint vertices
	if (node_hit_id != -1)
	{
		update_constraint();
	}
}

int geom_store::is_node_hit(glm::vec2& loc)
{
	// Return the node id of node which is clicked

	// Covert mouse location to screen location
	int max_dim = window_width > window_height ? window_width : window_height;

	// Transform the mouse location to openGL screen coordinates
	float screen_x = 2.0f * ((loc.x - (window_width * 0.5f)) / max_dim);
	float screen_y = 2.0f * (((window_height * 0.5f) - loc.y) / max_dim);


	// Nodal location
	glm::mat4 scaling_matrix = glm::mat4(1.0) * zoom_scale;
	scaling_matrix[3][3] = 1.0f;

	glm::mat4 scaledModelMatrix = scaling_matrix * modelMatrix;

	// Loop through all nodes in map and update min and max values
	for (auto it = nodeMap.begin(); it != nodeMap.end(); ++it)
	{
		const auto& node = it->second.node_pt;
		glm::vec4 finalPosition = scaledModelMatrix * glm::vec4(node.x, node.y, 0, 1.0f) * panTranslation;

		float node_position_x = finalPosition.x;
		float node_position_y = finalPosition.y;

		if ((((node_position_x - screen_x) * (node_position_x - screen_x)) +
			((node_position_y - screen_y) * (node_position_y - screen_y))) < (16 * node_circle_radii * node_circle_radii))
		{
			// Return the id of the node
			// 4 x Radius is the threshold of hit (2 * Diameter)
			return it->first;
		}
	}

	// None found
	return -1;
}

void geom_store::update_constraint()
{
	// Update the constraint
	if (constraintMap.constraint_count != 0)
	{
		// Constraint vertices
		const unsigned int constraint_vertex_count = 4 * 12 * constraintMap.constraint_count;
		float* constraint_vertices = new float[constraint_vertex_count];

		unsigned int constraint_indices_count = 6 * constraintMap.constraint_count;
		unsigned int* constraint_vertex_indices = new unsigned int[constraint_indices_count];

		unsigned int constraint_v_index = 0;
		unsigned int constraint_i_index = 0;

		for (auto& constraint : constraintMap.c_data)
		{
			// Add the constraint point
			set_constraint_vertices(constraint_vertices, constraint_v_index, constraint.second.node, constraint.second.constraint_angle, unsigned int(constraint.second.constraint_type));

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

}

void geom_store::update_load()
{
	// Update the load
	load_value_labels.init(&main_font);

	if (loadMap.load_count != 0)
	{
		// Load Arrow Head vertices
		const unsigned int load_arrowhead_vertex_count = 3 * 9 * loadMap.load_count;
		float* load_arrowhead_vertices = new float[load_arrowhead_vertex_count];

		// Load Arrow Head indices
		unsigned int load_arrowhead_indices_count = 3 * loadMap.load_count;
		unsigned int* load_arrowhead_vertex_indices = new unsigned int[load_arrowhead_indices_count];

		unsigned int load_arrowhead_v_index = 0;
		unsigned int load_arrowhead_i_index = 0;

		// Load Arrow tail vertices
		const unsigned int load_arrowtail_vertex_count = 2 * 9 * loadMap.load_count;
		float* load_arrowtail_vertices = new float[load_arrowtail_vertex_count];

		// Load Arrow tail indices
		unsigned int load_arrowtail_indices_count = 2 * loadMap.load_count;
		unsigned int* load_arrowtail_vertex_indices = new unsigned int[load_arrowtail_indices_count];

		unsigned int load_arrowtail_v_index = 0;
		unsigned int load_arrowtail_i_index = 0;

		// Load Max
		float load_max = 0.0f;

		// Find the load maximum
		for (auto& load : loadMap.l_data)
		{
			if (load_max < std::abs(load.second.load_value))
			{
				load_max = std::abs(load.second.load_value);
			}
		}

		// Set all the load labels
		for (auto& load : loadMap.l_data)
		{
			// Update the load Labels
			load_data load_val = load.second;

			glm::vec3 temp_color = glm::vec3(1.0f);
			std::string	temp_str = std::to_string(load_val.load_value);
			float load_angle = load_val.load_angle;


			glm::vec2 load_endpt = glm::vec2(0,
				-20.0f * (load_val.load_value / load_max) * (node_circle_radii / geom_scale));

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

			load_value_labels.add_text(temp_str.c_str(), node_value.node_pt, temp_color, geom_scale, load_angle_rad, font_size, true);
		}

		load_value_labels.set_buffers();


		for (auto& load : loadMap.l_data)
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
}

void geom_store::set_constraint_vertices(float* constraint_vertices, unsigned int& constraint_v_index, nodes_store* node, float constraint_angle, unsigned int constraint_type)
{
	// Set the Constraint vertices
	float constraint_size = (6.0f * node_circle_radii) / geom_scale;

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
	constraint_vertices[constraint_v_index + 6] = node_value.default_color.x;
	constraint_vertices[constraint_v_index + 7] = node_value.default_color.y;
	constraint_vertices[constraint_v_index + 8] = node_value.default_color.z;

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
	constraint_vertices[constraint_v_index + 6] = node_value.default_color.x;
	constraint_vertices[constraint_v_index + 7] = node_value.default_color.y;
	constraint_vertices[constraint_v_index + 8] = node_value.default_color.z;

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
	constraint_vertices[constraint_v_index + 6] = node_value.default_color.x;
	constraint_vertices[constraint_v_index + 7] = node_value.default_color.y;
	constraint_vertices[constraint_v_index + 8] = node_value.default_color.z;

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
	constraint_vertices[constraint_v_index + 6] = node_value.default_color.x;
	constraint_vertices[constraint_v_index + 7] = node_value.default_color.y;
	constraint_vertices[constraint_v_index + 8] = node_value.default_color.z;

	// Set the Texture co-ordinates
	constraint_vertices[constraint_v_index + 9] = 0.0f;
	constraint_vertices[constraint_v_index + 10] = 1.0f;

	// Texture type
	constraint_vertices[constraint_v_index + 11] = constraint_type;

	// Increment
	constraint_v_index = constraint_v_index + 12;
}

void geom_store::set_constraint_indices(unsigned int* constraint_vertex_indices, unsigned int& constraint_i_index)
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

void geom_store::set_load_arrowhead_vertices(float* load_arrowhead_vertices, unsigned int& load_arrowhead_v_index, nodes_store* node, float load_angle, float load_value)
{
	// Load arrow head vertices
	float load_arrowhead_size = (6.0f * node_circle_radii) / geom_scale;

	// Rotate the corner points
	glm::vec2 arrow_pt = glm::vec2(0, -(node_circle_radii / geom_scale)); // 0 0
	glm::vec2 arrow_hd_left = glm::vec2(-1.5f * (node_circle_radii / geom_scale), -5.0f * (node_circle_radii / geom_scale)); // -1 1
	glm::vec2 arrow_hd_right = glm::vec2(1.5f * (node_circle_radii / geom_scale), -5.0f * (node_circle_radii / geom_scale)); // 1 1

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

	// Set the node color
	load_arrowhead_vertices[load_arrowhead_v_index + 6] = node_value.default_color.x;
	load_arrowhead_vertices[load_arrowhead_v_index + 7] = node_value.default_color.y;
	load_arrowhead_vertices[load_arrowhead_v_index + 8] = node_value.default_color.z;

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

	// Set the node color
	load_arrowhead_vertices[load_arrowhead_v_index + 6] = node_value.default_color.x;
	load_arrowhead_vertices[load_arrowhead_v_index + 7] = node_value.default_color.y;
	load_arrowhead_vertices[load_arrowhead_v_index + 8] = node_value.default_color.z;

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

	// Set the node color
	load_arrowhead_vertices[load_arrowhead_v_index + 6] = node_value.default_color.x;
	load_arrowhead_vertices[load_arrowhead_v_index + 7] = node_value.default_color.y;
	load_arrowhead_vertices[load_arrowhead_v_index + 8] = node_value.default_color.z;

	// Increment
	load_arrowhead_v_index = load_arrowhead_v_index + 9;
}

void geom_store::set_load_arrowhead_indices(unsigned int* load_arrowhead_vertex_indices, unsigned int& load_arrowhead_i_index)
{
	// Load arrow head indices
	// Triangle 0,1,2
	load_arrowhead_vertex_indices[load_arrowhead_i_index + 0] = load_arrowhead_i_index + 0;
	load_arrowhead_vertex_indices[load_arrowhead_i_index + 1] = load_arrowhead_i_index + 1;
	load_arrowhead_vertex_indices[load_arrowhead_i_index + 2] = load_arrowhead_i_index + 2;

	// Increment
	load_arrowhead_i_index = load_arrowhead_i_index + 3;
}

void geom_store::set_load_arrowtail_vertices(float* load_arrowtail_vertices, unsigned int& load_arrowtail_v_index, nodes_store* node, float load_angle, float load_value, float load_max)
{

	int load_sign = load_value > 0 ? 1 : -1;
	// Rotate the corner points
	glm::vec2 arrow_tail_startpt = glm::vec2(0, -2.0f * load_sign * (node_circle_radii / geom_scale)); // 0 0
	glm::vec2 arrow_tail_endpt = glm::vec2(0, -20.0f * (load_value / load_max) * (node_circle_radii / geom_scale)); // -1 1

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

	// Set the node color
	load_arrowtail_vertices[load_arrowtail_v_index + 6] = node_value.default_color.x;
	load_arrowtail_vertices[load_arrowtail_v_index + 7] = node_value.default_color.y;
	load_arrowtail_vertices[load_arrowtail_v_index + 8] = node_value.default_color.z;

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

	// Set the node color
	load_arrowtail_vertices[load_arrowtail_v_index + 6] = node_value.default_color.x;
	load_arrowtail_vertices[load_arrowtail_v_index + 7] = node_value.default_color.y;
	load_arrowtail_vertices[load_arrowtail_v_index + 8] = node_value.default_color.z;

	// Increment
	load_arrowtail_v_index = load_arrowtail_v_index + 9;
}

void geom_store::set_load_arrowtail_indices(unsigned int* load_arrowtail_vertex_indices, unsigned int& load_arrowtail_i_index)
{
	// Load arrow tail indices
	// Line 0,1
	load_arrowtail_vertex_indices[load_arrowtail_i_index + 0] = load_arrowtail_i_index + 0;
	load_arrowtail_vertex_indices[load_arrowtail_i_index + 1] = load_arrowtail_i_index + 1;

	// Increment
	load_arrowtail_i_index = load_arrowtail_i_index + 2;
}

void geom_store::create_geometry_labels()
{
	main_font.create_atlas();

	// Create the labels
	node_id_labels.init(&main_font);
	line_id_labels.init(&main_font);
	node_coord_labels.init(&main_font);
	line_length_labels.init(&main_font);
	//load_value_labels.init(&main_font);

	glm::vec3 temp_color;
	std::string temp_str;

	// Add the node texts
	for (auto& node : nodeMap)
	{
		glm::vec2 node_pt = node.second.node_pt;

		// Add the node id
		temp_color = glm::vec3(1.0f);
		temp_str = std::to_string(node.first);


		node_id_labels.add_text(temp_str.c_str(), node_pt, temp_color, geom_scale, 0.0f, font_size, true);

		// Add the node coord
		temp_color = glm::vec3(1.0f);
		temp_str = "(" + std::to_string(node_pt.x) + ", " + std::to_string(node_pt.y) + ")";

		node_coord_labels.add_text(temp_str.c_str(), node_pt, temp_color, geom_scale, 0, font_size, false);
	}

	// Add the line texts
	glm::vec2 line_mid_pt;
	float line_angle;
	float line_length;

	for (auto& line : lineMap)
	{
		// Add the line id
		glm::vec2 start_pt = line.second.startNode.node_pt;
		glm::vec2 end_pt = line.second.endNode.node_pt;

		// Calculate the midpoint of the line segment
		glm::vec2 line_mid_pt = glm::vec2((start_pt.x + end_pt.x) * 0.5f, (start_pt.y + end_pt.y) * 0.5f);

		// Calculate the angle between the line segment and the x-axis
		float line_angle = atan2(end_pt.y - start_pt.y, end_pt.x - start_pt.x);

		// Calculate the length of the line segment
		float line_length = sqrt(pow(end_pt.x - start_pt.x, 2) + pow(end_pt.y - start_pt.y, 2));

		temp_color = glm::vec3(1.0f);
		temp_str = "[" + std::to_string(line.first) + "]";

		line_id_labels.add_text(temp_str.c_str(), line_mid_pt, temp_color, geom_scale, line_angle, font_size, true);

		// std::cout << line.first << "->" << line_angle << std::endl;

		// Add the Line Length
		temp_color = glm::vec3(1.0f);
		temp_str = std::to_string(line_length);

		line_length_labels.add_text(temp_str.c_str(), line_mid_pt, temp_color, geom_scale, line_angle, font_size, false);
	}

	// Set all the buffers
	node_id_labels.set_buffers();
	line_id_labels.set_buffers();
	node_coord_labels.set_buffers();
	line_length_labels.set_buffers();

}