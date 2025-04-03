#include "penalty_solver.h"

penalty_solver::penalty_solver()
{
	// Empty constructor
}

penalty_solver::~penalty_solver()
{
	// Empty destructor
}

void penalty_solver::solve_start(nodes_store_list* nodes, 
	lines_store_list* lines, 
	mconstraints* cnsts, 
	mloads* loads, 
	std::unordered_map<int, material_data>* mdatas, 
	mloads& reaction_x, 
	mloads& reaction_y, 
	solver_window* fe_window)
{
	// Main Solver Start
	fe_window->is_analysis_complete = false;

	// Check the model
	fe_window->log_buffer = "Checking the model ...... \n";
	fe_window->log_buffer.append(". \n");
	fe_window->log_buffer.append(". \n");

	// Number of nodes
	if (nodes->node_count == 0)
	{
		fe_window->log_buffer.append("No Nodes in the model ....... Analysis Failed! \n");
		return;
	}
	else
	{
		fe_window->log_buffer.append("Number of nodes = ");
		fe_window->log_buffer.append(std::to_string(nodes->node_count));
		fe_window->log_buffer.append("\n");
	}

	// Number of elements
	if (lines->line_count == 0)
	{
		fe_window->log_buffer.append("No Elements in the model ....... Analysis Failed! \n");
		return;
	}
	else
	{
		fe_window->log_buffer.append("Number of elements = ");
		fe_window->log_buffer.append(std::to_string(lines->line_count));
		fe_window->log_buffer.append("\n");
	}

	// Number of constraints
	if (cnsts->constraint_count < 2)
	{
		fe_window->log_buffer.append("Model has less than 2 constraints (Rigid body motion detected) ....... Analysis Failed! \n");
		return;
	}
	else
	{
		fe_window->log_buffer.append("Number of constraints = ");
		fe_window->log_buffer.append(std::to_string(cnsts->constraint_count));
		fe_window->log_buffer.append("\n");
	}

	// Number of loads
	if (loads->load_count < 1)
	{
		fe_window->log_buffer.append("No loads in the model ....... Analysis Failed! \n");
		return;
	}
	else
	{
		fe_window->log_buffer.append("Number of loads = ");
		fe_window->log_buffer.append(std::to_string(loads->load_count));
		fe_window->log_buffer.append("\n");
	}

	fe_window->log_buffer.append("Analysis started ........ \n");
	// Create a node ID map (assuming all the nodes are ordered and numbered from 0,1,2...n)
	int i = 0;
	nodeid_map.clear();
	for (auto& nd : nodes->nodeMap)
	{
		nodeid_map[nd.first] = i;
		i++;
	}

	fe_window->log_buffer.append("1. Node ID map created \n");

	// Create a file to keep track of matrices
	std::ofstream output_file;
	output_file.open("fe_matrices_penalty.txt");

	//____________________________________________________________________________________________________________________
	int numDOF = nodes->node_count * 2; // Number of degrees of freedom (2 DOFs per node)


	//____________________________________________________________________________________________________________________
	// Global Stiffness Matrix
	Eigen::MatrixXd globalStiffnessMatrix(numDOF, numDOF);
	globalStiffnessMatrix.setZero();


	get_global_stiffness_matrix(globalStiffnessMatrix, lines, mdatas, cnsts, output_file);

	fe_window->log_buffer.append("2. Global Stiffness Matrix created \n");

	//____________________________________________________________________________________________________________________
	// Global force matrix
	Eigen::VectorXd globalForceMatrix(numDOF);
	globalForceMatrix.setZero();

	get_global_force_matrix(globalForceMatrix, nodes, loads, output_file);

	fe_window->log_buffer.append("3. Global Force Matrix created \n");

	//____________________________________________________________________________________________________________________
	// Apply Boundary condition





}



void penalty_solver::get_global_stiffness_matrix(Eigen::MatrixXd& globalStiffnessMatrix, 
	lines_store_list* lines, std::unordered_map<int, material_data>* mdatas, mconstraints* cnsts, std::ofstream& output_file)
{
	this->penaltyK = 0.0;

	// Create the global stiffness matrix
	for (auto& ln_m : lines->lineMap)
	{
		// Create the element stiffness matrix
		lines_store ln = ln_m.second;
		material_data mdata = (*mdatas)[ln.material_id];

		// Create a matrix for element striffness matrix
		Eigen::Matrix4d elementStiffnessMatrix = Eigen::Matrix4d::Zero();;

		get_element_stiffness_matrix(elementStiffnessMatrix, ln, mdata, cnsts, output_file);

		// Get the Node ID
		int sn_id = nodeid_map[ln.startNode.node_id]; // get the ordered map of the start node ID
		int en_id = nodeid_map[ln.endNode.node_id]; // get the ordered map of the end node ID

		globalStiffnessMatrix.block<2, 2>(sn_id * 2, sn_id * 2) += elementStiffnessMatrix.block<2, 2>(0, 0);
		globalStiffnessMatrix.block<2, 2>(sn_id * 2, en_id * 2) += elementStiffnessMatrix.block<2, 2>(0, 2);
		globalStiffnessMatrix.block<2, 2>(en_id * 2, sn_id * 2) += elementStiffnessMatrix.block<2, 2>(2, 0);
		globalStiffnessMatrix.block<2, 2>(en_id * 2, en_id * 2) += elementStiffnessMatrix.block<2, 2>(2, 2);
	}

	// Set the penalty stiffness
	if (this->penaltyK > 0.000001)
	{
		this->penaltyK = this->penaltyK * 1000000.0;
	}
	else
	{
		this->penaltyK = 1000000.0;
	}

	if (print_matrix == true)
	{
		// Print the Global Stiffness matrix
		output_file << "Global Stiffness Matrix" << std::endl;
		output_file << globalStiffnessMatrix << std::endl;
		output_file << std::endl;
	}

}



void penalty_solver::get_element_stiffness_matrix(Eigen::Matrix4d& elementStiffnessMatrix, 
	lines_store& ln, material_data& mdata, mconstraints* cnsts, std::ofstream& output_file)
{
	// Create a element stiffness matrix
	// Compute the differences in x and y coordinates
	double dx = ln.endNode.node_pt.x - ln.startNode.node_pt.x;
	double dy = -1.0 * (ln.endNode.node_pt.y - ln.startNode.node_pt.y);

	// Compute the length of the truss element
	double eLength = std::sqrt((dx * dx) + (dy * dy));

	// Compute the direction cosines
	double Lcos = (dx / eLength);
	double Msin = (dy / eLength);

	// Compute the stiffness factor
	double k1 = 0.0;
	
	if (mdata.material_id == 0)
	{
		// Rigid element
		k1 = 0.0;

	}
	else
	{
		// Flexible element
		k1 = (mdata.cs_area * mdata.youngs_mod) / eLength;

		// Find the maximum stiffness
		this->penaltyK = std::max(this->penaltyK, k1);
	}
	

	//Stiffness matrix components
	double v1 = k1 * std::pow(Lcos, 2);
	double v2 = k1 * std::pow(Msin, 2);
	double v3 = k1 * (Lcos * Msin);

	// Create the Element stiffness matrix
	elementStiffnessMatrix.row(0) = Eigen::RowVector4d(v1, v3, -v1, -v3);
	elementStiffnessMatrix.row(1) = Eigen::RowVector4d(v3, v2, -v3, -v2);
	elementStiffnessMatrix.row(2) = Eigen::RowVector4d(-v1, -v3, v1, v3);
	elementStiffnessMatrix.row(3) = Eigen::RowVector4d(-v3, -v2, v3, v2);


	if (print_matrix == true)
	{
		// Print the Element Stiffness matrix
		output_file << "Member (" << ln.startNode.node_id << " -> " << ln.endNode.node_id << ")" << std::endl;
		output_file << elementStiffnessMatrix << std::endl;
		output_file << std::endl;
	}

}



void penalty_solver::get_global_force_matrix(Eigen::VectorXd& globalForceMatrix,
	nodes_store_list* nodes, mloads* loads, std::ofstream& output_file)
{
	// Create a global force matrix
	for (auto& nd_l : nodes->nodeMap)
	{
		// Get the node data
		nodes_store nd = nd_l.second;
		int nd_map = nodeid_map[nd.node_id]; // get the ordered map of the node ID

		if (loads->l_data.find(nd.node_id) != loads->l_data.end())
		{
			// Nodes have loads
			load_data ld = loads->l_data[nd.node_id];
			double load_val = ld.load_value; // Load value
			double load_angle_rad = ld.load_angle * (m_pi / 180.0f);

			double f_x = load_val * std::cos(load_angle_rad);
			double f_y = load_val * std::sin(load_angle_rad);

			globalForceMatrix((nd_map * 2) + 0, 0) += f_x;
			globalForceMatrix((nd_map * 2) + 1, 0) += f_y;
		}
		else
		{
			// Nodes doesn't have loads
			globalForceMatrix((nd_map * 2) + 0, 0) += 0.0;
			globalForceMatrix((nd_map * 2) + 1, 0) += 0.0;
		}
	}

	if (print_matrix == true)
	{
		// Print the Global Force matrix
		output_file << "Global Force Matrix" << std::endl;
		output_file << std::fixed << std::setprecision(6) << globalForceMatrix << std::endl;  // Set decimal precision to 6 
		output_file << std::endl;
	}

}



void penalty_solver::apply_SPC_boundary_conditions(Eigen::MatrixXd& globalStiffnessMatrix, Eigen::VectorXd& globalForceMatrix, int numDOF,
	nodes_store_list* nodes, lines_store_list* lines, mconstraints* cnsts, std::ofstream& output_file)
{

	// Apply boundary condition using Penalty method
	// Single point constraint (Pinned or Roller boundary condition)

	Eigen::MatrixXd global_penalty_SPC_StiffnessMatrix(numDOF, numDOF);
	global_penalty_SPC_StiffnessMatrix.setZero();


	Eigen::VectorXd global_penalty_SPC_ForceMatrix(numDOF);
	global_penalty_SPC_ForceMatrix.setZero();


	for (auto& nd_l : nodes->nodeMap)
	{
		// Get the node data
		nodes_store nd = nd_l.second;
		int nd_map = nodeid_map[nd.node_id]; // get the ordered map of the node ID

		if (cnsts->c_data.find(nd.node_id) != cnsts->c_data.end())
		{
			// Node have single point constraint
			constraint_data c_data = cnsts->c_data[nd.node_id];

			int constraint_type = c_data.constraint_type; // constraint type
			double constraint_angle_rad = (c_data.constraint_angle - 90.0f)* (m_pi / 180.0f); // constraint angle
			double support_Lcos = std::cos(constraint_angle_rad); // cosine of support inclination
			double support_Msin = std::sin(constraint_angle_rad); // sine of support inclination

			// Penalty single point constraint stiffness matrix
			Eigen::Matrix2d penalty_SPC_StiffnessMatrix;
			Eigen::Vector2d penalty_SPC_ForceMatrix;

			if (constraint_type == 0)
			{
				// Pinned support
				penalty_SPC_StiffnessMatrix.row(0) = Eigen::RowVector2d(support_Lcos, -1.0 * support_Msin);
				penalty_SPC_StiffnessMatrix.row(1) = Eigen::RowVector2d(support_Msin, support_Lcos);



							// Pin or Roller Support
				globalSupportInclinationMatrix.coeffRef((matrix_index * 2) + 0, (matrix_index * 2) + 0) = support_Lcos;
				globalSupportInclinationMatrix.coeffRef((matrix_index * 2) + 0, (matrix_index * 2) + 1) = -1.0 * support_Msin;

				globalSupportInclinationMatrix.coeffRef((matrix_index * 2) + 1, (matrix_index * 2) + 0) = support_Msin;
				globalSupportInclinationMatrix.coeffRef((matrix_index * 2) + 1, (matrix_index * 2) + 1) = support_Lcos;

			}
			else if (constraint_type == 1)
			{
				// Roller support
				

			}

			double f_x = load_val * std::cos(load_angle_rad);
			double f_y = load_val * std::sin(load_angle_rad);

			globalForceMatrix((nd_map * 2) + 0, 0) += f_x;
			globalForceMatrix((nd_map * 2) + 1, 0) += f_y;
		}
		else
		{
			// Node doesn't have single point constraint
			globalForceMatrix((nd_map * 2) + 0, 0) += 0.0;
			globalForceMatrix((nd_map * 2) + 1, 0) += 0.0;
		}
	}





	// Multi point constraint (Rigid link)






}

