#include "geom_store.h"

geom_store::geom_store()
	: is_geometry_set(false),
	is_geometry_loaded(false)
{
	// Empty constructor
}

geom_store::~geom_store()
{
	// Destructor
	deleteResources();
}

void geom_store::init()
{
	// Initialize the geometry parameters
	geom_param.init();

	// Initialize the nodes and lines
	this->model_nodes.init(&geom_param);
	this->model_lines.init(&geom_param);

	// Initialize constraint and loads
	this->loadMap.init(&geom_param);
	this->constraintMap.init(&geom_param);
}

void geom_store::write_rawdata(std::ofstream& file)
{
	// Write all the nodes
	for (auto& node : model_nodes.nodeMap)
	{
		// Print the node details
		nodes_store nd_val = node.second;

		file << "node, "
			<< nd_val.node_id << ", "
			<< nd_val.node_pt.x << ", "
			<< nd_val.node_pt.y << std::endl;
	}

	// Write all the lines
	for (auto& line : model_lines.lineMap)
	{
		// Print the line details
		lines_store ln_val = line.second;

		file << "line, "
			<< ln_val.line_id << ", "
			<< ln_val.startNode.node_id << ", "
			<< ln_val.endNode.node_id << ", "
			<< ln_val.material_id << std::endl;
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
		material_data mat_d = mat.second;
		file << "mtrl, "
			<< mat_d.material_id << ", "
			<< mat_d.material_name << ", "
			<< mat_d.youngs_mod << ", "
			<< mat_d.mat_density << ", "
			<< mat_d.cs_area << std::endl;
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


	// Create a temporary variable to store the nodes
	nodes_store_list model_nodes;
	model_nodes.init(&geom_param);
	// Create an unordered_map to store lines with ID as key
	lines_store_list model_lines;
	model_lines.init(&geom_param);
	// Constraint data store
	mconstraints constraintMap;
	constraintMap.init(&geom_param);
	// Load data store
	mloads loadMap;
	loadMap.init(&geom_param);
	// Material data list
	std::unordered_map<int,material_data> mat_data;

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
			model_nodes.add_node(node_id, glm::vec3(x, y, 0.0f));
		}
		else if (type == "line")
		{
			int line_id = std::stoi(fields[1]); // line ID
			int start_node_id = std::stoi(fields[2]); // line id start node
			int end_node_id = std::stoi(fields[3]); // line id end node
			int material_id = std::stoi(fields[4]); // materail ID of the line

			// Add to line Map (Note that Nodes needed to be added before the start of line addition !!!!)
			model_lines.add_line(line_id, model_nodes.nodeMap[start_node_id], model_nodes.nodeMap[end_node_id], material_id);
		}
		else if (type == "cnst")
		{
			int cnst_nd_id = std::stoi(fields[1]); // constraint node ID
			int cnst_type = std::stoi(fields[2]); // constraint type 
			float cnst_angle = std::stof(fields[3]); // constraint angle

			// Add to constraint map
			constraintMap.add_constraint(cnst_nd_id, &model_nodes.nodeMap[cnst_nd_id], cnst_type, cnst_angle);
		}
		else if (type == "load")
		{
			int load_nd_id = std::stoi(fields[1]); // load node ID
			float load_val = std::stof(fields[2]); // load value
			float load_angle = std::stof(fields[3]); // load angle

			// Add to load map
			loadMap.add_load(load_nd_id, &model_nodes.nodeMap[load_nd_id], load_val, load_angle);
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
			mat_data[inpt_material.material_id]=inpt_material;
		}

		// Iterate line
		j++;
	}


	// Data loaded create the geometry

	if (model_nodes.node_count < 1 || model_lines.line_count < 1)
	{
		// No elements added
		return;
	}

	//Add the materail list
	mat_window->material_list = mat_data;

	// Re-instantitize geom_store object using the nodeMap and lineMap
	deleteResources();
	create_geometry(model_nodes, model_lines, constraintMap, loadMap);
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


	// Create a temporary variable to store the nodes
	nodes_store_list model_nodes;
	model_nodes.init(&geom_param);
	// Create a temporary variable to store the lines
	lines_store_list model_lines;
	model_lines.init(&geom_param);
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

				// Add to node store list
				model_nodes.add_node(node_id, glm::vec3(x, y, 0.0f));

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
				model_lines.add_line(line_id, model_nodes.nodeMap[start_node_id], model_nodes.nodeMap[end_node_id], 0);

				j++;
			}
		}

		// iterate line
		j++;
	}

	if (model_nodes.node_count < 1 || model_lines.line_count < 1)
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
	mat_window->material_list[inpt_material.material_id] = inpt_material;


	mconstraints constraintMap;
	constraintMap.init(&geom_param);
	mloads loadMap;
	loadMap.init(&geom_param);
	// Re-instantitize geom_store object using the nodeMap and lineMap
	deleteResources();
	create_geometry(model_nodes, model_lines, constraintMap, loadMap);
}

void geom_store::create_geometry(nodes_store_list& model_nodes,
	lines_store_list& model_lines,
	mconstraints& constraintMap,
	mloads& loadMap)
{

	// Initialize the model nodes and lines
	this->model_nodes = model_nodes;
	this->model_lines = model_lines;
	this->model_lines.add_node_list(&this->model_nodes.nodeMap);

	// this->constraintMap = constraintMap;
	// this->loadMap = loadMap;

	// Update the node pointers for loads and constraints
	// Loads
	this->loadMap.init(&geom_param);
	for (auto& ld : loadMap.l_data)
	{
		load_data l_data = ld.second;
		this->loadMap.add_load(l_data.node_id, &this->model_nodes.nodeMap[l_data.node_id], l_data.load_value, l_data.load_angle);
	}

	// Constraints
	this->constraintMap.init(&geom_param);
	for (auto& cnst : constraintMap.c_data)
	{
		constraint_data c_data = cnst.second;
		this->constraintMap.add_constraint(c_data.node_id, &this->model_nodes.nodeMap[c_data.node_id], c_data.constraint_type, c_data.constraint_angle);
	}

	// Set the boundary of the geometry
	std::pair<glm::vec3, glm::vec3> result = findMinMaxXY(model_nodes.nodeMap);
	geom_param.min_b = result.first;
	geom_param.max_b = result.second;
	geom_param.geom_bound = geom_param.max_b - geom_param.min_b;

	// Set the center of the geometry
	geom_param.center = findGeometricCenter(model_nodes.nodeMap);

	// Geometry is loaded
	is_geometry_loaded = true;

	set_geometry();

	// Set the buffers for constraints, loads and material IDs
	this->constraintMap.update_buffer();
	this->loadMap.update_buffer();
	this->model_lines.update_material_id_buffer();
}

void geom_store::add_window_ptr(options_window* op_window, material_window* mat_window, solver_window* fe_window)
{
	// Add the pointer of the windows (material and option window)
	this->op_window = op_window;
	this->mat_window = mat_window;
	this->fe_window = fe_window;
}

void geom_store::deleteResources()
{
	is_geometry_loaded = false;
	is_geometry_set = false;

	// Re-initialize the nodes and lines
	this->model_nodes.init(&geom_param);
	this->model_lines.init(&geom_param);

	// Re-initialize constraint and loads
	this->loadMap.init(&geom_param);
	this->constraintMap.init(&geom_param);

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

	// Set the buffer for nodes and lines
	model_nodes.set_buffer();
	model_lines.set_buffer();

	// Set the buffer for constraints
	constraintMap.set_buffer();
	loadMap.set_buffer();

	// Create shader
	std::filesystem::path currentDirPath = std::filesystem::current_path();
	std::filesystem::path parentPath = currentDirPath.parent_path();
	std::filesystem::path shadersPath = parentPath / "Truss_static_analysis_cpp/src/geometry_store/shaders";
	std::string parentString = shadersPath.string();
	std::cout << "Parent path: " << parentString << std::endl;

	// Text shader
	text_shader.create_shader((shadersPath.string() + "/text_vert_shader.vert").c_str(),
		(shadersPath.string() + "/text_frag_shader.frag").c_str());

	// Set texture uniform variables
	text_shader.setUniform("u_Texture", 0);

	// Geometry is set
	is_geometry_set = true;
}

void geom_store::paint_geometry()
{
	if (is_geometry_set == false)
		return;

	// Clean the back buffer and assign the new color to it
	glClear(GL_COLOR_BUFFER_BIT);

	// Analysis solution paint (Analysis complete !!)
	if (fe_window->is_show_window == true)
	{
		// Execute the open window event
		if (fe_window->execute_open == true)
		{
			update_model_transperency(true);
			// Opening event of solution window done
			fe_window->execute_open = false;
		}

		// Execute the Analysis
		if (fe_window->execute_solver == true)
		{
			fe_sol.solve_start(&model_nodes,&model_lines,&constraintMap,&loadMap,&mat_window->material_list,fe_window);
			// Analysis execution complete
			fe_window->execute_solver = false;
		}


		if (fe_window->is_analysis_complete == true)
		{
			// Analysis complete Paint the results

		}
	}

	// Execute the closing of solution window
	if (fe_window->execute_close == true)
	{
		update_model_transperency(false);
		// Closing event of solution window done
		fe_window->execute_close = false;
	}

	// Paint the model
	paint_model();
}

void geom_store::paint_model()
{
	// Paint the model

	model_lines.paint_lines();	// Paint the Lines
	
	model_nodes.paint_nodes(); // Paint the Nodes

	constraintMap.paint_constraints();	// Paint the constraints

	loadMap.paint_loads();// Paint the Loads

	// Paint the labels
	text_shader.Bind();

	if (op_window->is_show_nodenumber == true)
	{
		model_nodes.paint_node_ids();// Show node ids
	}

	if (op_window->is_show_nodecoord == true)
	{
		model_nodes.paint_node_coords();// Show node coordinate
	}

	if (op_window->is_show_linenumber == true)
	{
		model_lines.paint_line_ids();// Show line id
	}

	if (op_window->is_show_linelength == true)
	{
		model_lines.paint_line_length();// Show line length
	}

	if (op_window->is_show_loadvalue == true && loadMap.load_count != 0)
	{
		loadMap.paint_load_labels();// Show the load value
	}

	// Show the materials of line member
	if (mat_window->is_show_window == true)
	{
		if (mat_window->execute_delete_materialid != -1)
		{
			// Delete material
			update_delete_material(mat_window->execute_delete_materialid);
			mat_window->execute_delete_materialid = -1;
		}
		// Show the material ID
		model_lines.paint_line_material_id();
	}

	text_shader.UnBind();
}

void geom_store::update_model_transperency(bool is_transparent)
{
	if (is_transparent == true)
	{
		// Set the transparency value
		text_shader.setUniform("transparency", 0.1f);
		geom_param.geom_transparency = 0.2f;
	}
	else
	{
		// remove transparency
		text_shader.setUniform("transparency", 0.7f);
		geom_param.geom_transparency = 1.0f;
	}

	// Update the model matrices
	model_lines.update_geometry_matrices(false, false, false, true);
	model_nodes.update_geometry_matrices(false, false, false, true);
	constraintMap.update_geometry_matrices(false, false, false, true);
	loadMap.update_geometry_matrices(false, false, false, true);
}

void geom_store::set_model_matrix()
{
	// Set the model matrix for the model shader
	// Find the scale of the model (with 0.9 being the maximum used)
	int max_dim = geom_param.window_width > geom_param.window_height ? geom_param.window_width : geom_param.window_height;

	float normalized_screen_width = 1.8f * (float(geom_param.window_width) / float(max_dim));
	float normalized_screen_height = 1.8f * (float(geom_param.window_height) / float(max_dim));


	geom_param.geom_scale = std::min(normalized_screen_width / geom_param.geom_bound.x,
		normalized_screen_height / geom_param.geom_bound.y);

	// Translation
	glm::vec3 geom_translation = glm::vec3(-1.0f * (geom_param.max_b.x + geom_param.min_b.x) * 0.5f * geom_param.geom_scale,
		-1.0f * (geom_param.max_b.y + geom_param.min_b.y) * 0.5f * geom_param.geom_scale,
		-1.0f * (geom_param.max_b.z + geom_param.min_b.z) * 0.5f * geom_param.geom_scale);

	glm::mat4 g_transl = glm::translate(glm::mat4(1.0f), geom_translation);

	geom_param.modelMatrix = g_transl * glm::scale(glm::mat4(1.0f), glm::vec3(geom_param.geom_scale));


	model_lines.update_geometry_matrices(true, false, false,true);
	model_nodes.update_geometry_matrices(true, false, false, true);
	constraintMap.update_geometry_matrices(true, false, false, true);
	loadMap.update_geometry_matrices(true, false, false, true);

	text_shader.setUniform("transparency", 0.7f);
	text_shader.setUniform("modelMatrix", geom_param.modelMatrix, false);
}

void geom_store::updateWindowDimension(const int& window_width, const int& window_height)
{
	// Update the window dimension
	this->geom_param.window_width = window_width;
	this->geom_param.window_height = window_height;

	if (is_geometry_set == true)
	{
		// Update the model matrix
		set_model_matrix();
		// !! Zoom to fit operation during window resize is handled in mouse event class !!
	}
}

void geom_store::zoomfit_geometry()
{
	if (is_geometry_set == false)
		return;

	// Set the pan translation matrix
	geom_param.panTranslation = glm::mat4(1.0f);
	text_shader.setUniform("panTranslation", geom_param.panTranslation, false);

	// Set the zoom matrix
	geom_param.zoom_scale = 1.0f;

	model_lines.update_geometry_matrices(false, true, true, false);
	model_nodes.update_geometry_matrices(false, true, true, false);
	constraintMap.update_geometry_matrices(false, true, true, false);
	loadMap.update_geometry_matrices(false, true, true, false);

	text_shader.setUniform("zoomscale", geom_param.zoom_scale);
}

void geom_store::pan_geometry(glm::vec2& transl)
{
	if (is_geometry_set == false)
		return;

	// Pan the geometry
	geom_param.panTranslation = glm::mat4(1.0f);

	geom_param.panTranslation[0][3] = -1.0f * transl.x;
	geom_param.panTranslation[1][3] = transl.y;

	model_lines.update_geometry_matrices(false, true, false,false);
	model_nodes.update_geometry_matrices(false, true, false, false);
	constraintMap.update_geometry_matrices(false, true, false, false);
	loadMap.update_geometry_matrices(false, true, false, false);

	text_shader.setUniform("panTranslation", geom_param.panTranslation, false);
}

void geom_store::zoom_geometry(float& z_scale)
{
	if (is_geometry_set == false)
		return;

	geom_param.zoom_scale = z_scale;

	// Zoom the geometry
	model_lines.update_geometry_matrices(false, false, true, false);
	model_nodes.update_geometry_matrices(false, false, true, false);
	constraintMap.update_geometry_matrices(false, false, true, false);
	loadMap.update_geometry_matrices(false, false, true, false);

	text_shader.setUniform("zoomscale", geom_param.zoom_scale);
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
			node_hit_id = model_nodes.is_node_hit(loc);
			if (node_hit_id != -1)
			{
				loadMap.add_load(node_hit_id, &model_nodes.nodeMap[node_hit_id], load_value, load_angle);
				// Node hit == True
				// std::cout << "Node Hit: " << node_hit_id << std::endl;
			}
		}
		else
		{
			// Remove load
			node_hit_id = model_nodes.is_node_hit(loc);
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
		loadMap.update_buffer();
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
			node_hit_id = model_nodes.is_node_hit(loc);
			if (node_hit_id != -1)
			{
				constraintMap.add_constraint(node_hit_id, &model_nodes.nodeMap[node_hit_id], constraint_type, constraint_angle);
				// Node hit == True
				// std::cout << "Node Hit: " << node_hit_id << std::endl;
			}
		}
		else
		{
			// Remove constraints
			node_hit_id = model_nodes.is_node_hit(loc);
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
		constraintMap.update_buffer();
	}
}

void geom_store::set_line_material(glm::vec2& loc)
{
	// Set line material
	int line_hit_id = -1;

	if (is_geometry_set == true)
	{
		// Line Materail
		line_hit_id = model_lines.is_line_hit(loc);
		if (line_hit_id != -1)
		{
			int selected_material_option = mat_window->selected_material_option;
			model_lines.lineMap[line_hit_id].material_id = mat_window->material_list[selected_material_option].material_id;
		}
	}

	// Update the material ID label
	if (line_hit_id != -1)
	{
		model_lines.update_material_id_buffer();
	}
}

void geom_store::update_delete_material(int& del_material_id)
{
	bool is_del_material_found = false;

	// Delete the material
	for (int i = 0; i < model_lines.lineMap.size(); i++)
	{
		if (model_lines.lineMap[i].material_id == del_material_id)
		{
			// Delete material is removed and the material ID of that element to 0
			model_lines.lineMap[i].material_id = 0;
			is_del_material_found = true;
		}
	}

	// Update the material ID label
	if (is_del_material_found == true)
	{
		model_lines.update_material_id_buffer();
	}
}