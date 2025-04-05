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
	// Global Penalty Stiffness matrix
	Eigen::MatrixXd globalPenaltyStiffnessMatrix(numDOF, numDOF);
	globalPenaltyStiffnessMatrix.setZero();

	get_boundary_condition_penalty_matrix(globalPenaltyStiffnessMatrix, numDOF, nodes, lines, cnsts, output_file);

	fe_window->log_buffer.append("4. Global Penalty Stiffness Matrix created \n");


	//____________________________________________________________________________________________________________________
	// Penalty Augmentation of global stiffness matrix

	globalStiffnessMatrix = globalStiffnessMatrix + globalPenaltyStiffnessMatrix;

	//____________________________________________________________________________________________________________________
	// Global Displacement matrix
	Eigen::VectorXd globalDisplacementMatrix(numDOF);
	globalDisplacementMatrix.setZero();


	// Solve using Partial Pivot LU decomposition
	globalDisplacementMatrix = globalStiffnessMatrix.lu().solve(globalForceMatrix);


	if (print_matrix == true)
	{
		// Print the Reduced Global Displacement matrix
		output_file << "Global Displacement Matrix" << std::endl;
		output_file << globalDisplacementMatrix << std::endl;
		output_file << std::endl;
	}

	fe_window->log_buffer.append("5. LU Decomposition Matrix solution for Displacement completed \n");

	//____________________________________________________________________________________________________________________
	// Global Resultant matrix
	Eigen::VectorXd globalResultantMatrix(numDOF);
	globalResultantMatrix.setZero();

	globalResultantMatrix = (globalStiffnessMatrix * globalDisplacementMatrix) - globalForceMatrix;

	if (print_matrix == true)
	{
		// Print the Global Resultant matrix
		output_file << "Global Resultant Matrix" << std::endl;
		output_file << globalResultantMatrix << std::endl;
		output_file << std::endl;
	}

	fe_window->log_buffer.append("6. Resultant forces calculated \n");


	//____________________________________________________________________________________________________________________
	// Map the analysis Results
	bool is_map_success = false;

	map_analysis_results(globalDisplacementMatrix,
		globalResultantMatrix,
		nodes,
		lines,
		cnsts,
		loads,
		reaction_x,
		reaction_y,
		mdatas,
		is_map_success,
		output_file);


	if (is_map_success == false)
	{
		output_file << "Resultant force is Zero or Max Displacement is Zero" << std::endl;
		output_file << "Analysis failed ........ " << std::endl;
		fe_window->log_buffer.append("Resultant force is Zero or Max Displacement is Zero \n");
		fe_window->log_buffer.append("Analysis failed ........ \n");
		fe_window->log_buffer.append("Reason might be material properties are not set properly, connectivity of elements, Rigid body motion due to improper boundary condition \n");
		return;
	}

	fe_window->log_buffer.append("Solve success ........ \n");

	fe_window->log_buffer.append("Results are mapped to elements \n");

	// Set the node buffers
	nodes->set_defl_buffer();
	nodes->update_geometry_matrices(true, true, true, true);

	// Set the line buffers
	lines->set_defl_buffer();
	lines->set_mforce_buffer();
	lines->update_geometry_matrices(true, true, true, true);

	fe_window->is_analysis_complete = true;


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
		this->penaltyK = this->penaltyK * 10000000.0;
	}
	else
	{
		this->penaltyK = 10000000.0;
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



void penalty_solver::get_boundary_condition_penalty_matrix(Eigen::MatrixXd& globalPenaltyStiffnessMatrix, int numDOF,
	nodes_store_list* nodes, lines_store_list* lines, mconstraints* cnsts, std::ofstream& output_file)
{

	// Apply boundary condition using Penalty method
	// Single point constraint (Pinned or Roller boundary condition)

	Eigen::MatrixXd global_penalty_SPC_AMatrix(numDOF, 0); // Start with zero columns


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
			double constraint_angle_rad = (c_data.constraint_angle - 90.0f) * (m_pi / 180.0f); // constraint angle
			double support_Lcos = std::cos(constraint_angle_rad); // cosine of support inclination
			double support_Msin = std::sin(constraint_angle_rad); // sine of support inclination

			// Penalty single point constraint A vector
			Eigen::VectorXd penalty_SPC_AxVector(numDOF);
			penalty_SPC_AxVector.setZero();

			Eigen::VectorXd penalty_SPC_AyVector(numDOF);
			penalty_SPC_AyVector.setZero();


			if (constraint_type == 0)
			{
				// Pinned support
				penalty_SPC_AxVector[(nd_map * 2) + 0] = 1.0;
				penalty_SPC_AxVector[(nd_map * 2) + 1] = 0.0;

				penalty_SPC_AyVector[(nd_map * 2) + 0] = 0.0;
				penalty_SPC_AyVector[(nd_map * 2) + 1] = 1.0;

			}
			else if (constraint_type == 1)
			{
				// Roller support
				penalty_SPC_AxVector[(nd_map * 2) + 0] = support_Msin * support_Msin;
				penalty_SPC_AxVector[(nd_map * 2) + 1] = support_Lcos * support_Msin;

				penalty_SPC_AyVector[(nd_map * 2) + 0] = support_Lcos * support_Msin;
				penalty_SPC_AyVector[(nd_map * 2) + 1] = support_Lcos * support_Lcos;


				//penalty_SPC_AxVector[(nd_map * 2) + 0] = support_Lcos * support_Lcos ;
				//penalty_SPC_AxVector[(nd_map * 2) + 1] = support_Lcos * support_Msin;

				//penalty_SPC_AyVector[(nd_map * 2) + 0] = support_Lcos * support_Msin;
				//penalty_SPC_AyVector[(nd_map * 2) + 1] = support_Msin * support_Msin;


			}

			// **Expand A_matrix by adding a new column**
			int currentCols = global_penalty_SPC_AMatrix.cols();
			global_penalty_SPC_AMatrix.conservativeResize(numDOF, currentCols + 1); // Add one column
			global_penalty_SPC_AMatrix.col(currentCols) = penalty_SPC_AxVector;       // Insert the new vector

			currentCols = global_penalty_SPC_AMatrix.cols();
			global_penalty_SPC_AMatrix.conservativeResize(numDOF, currentCols + 1); // Add one column
			global_penalty_SPC_AMatrix.col(currentCols) = penalty_SPC_AyVector;       // Insert the new vector

		}

	}

	// Find the global penalty stiffness matrix
	globalPenaltyStiffnessMatrix = this->penaltyK * (global_penalty_SPC_AMatrix * global_penalty_SPC_AMatrix.transpose());


	if (print_matrix == true)
	{
		// Print the Global Penalty Stiffness matrix
		output_file << "Global Penalty Stiffness Matrix" << std::endl;
		output_file << std::fixed << std::setprecision(6) << globalPenaltyStiffnessMatrix << std::endl;  // Set decimal precision to 6 
		output_file << std::endl;
	}


}



void penalty_solver::map_analysis_results(Eigen::VectorXd& globalDisplacementMatrix,
	Eigen::VectorXd& globalResultantMatrix,
	nodes_store_list* nodes,
	lines_store_list* lines,
	mconstraints* cnsts,
	mloads* loads,
	mloads& reaction_x,
	mloads& reaction_y,
	std::unordered_map<int, material_data>* mdatas,
	bool& is_map_success,
	std::ofstream& output_file)
{

	double max_memberforce = 0.0;
	double max_memberstress = 0.0;

	double constraint_angle_rad = 0.0;
	double support_Lcos = 0.0;
	double support_Msin = 0.0;

	// Main function to fix the results displacement, resultant force, member force and member stress
	for (auto& ln_m : lines->lineMap)
	{
		// Get the line member
		lines_store ln = ln_m.second;

		// Get the matrix index of both start and end node
		int SN_matrix_index = nodeid_map[ln.startNode.node_id];
		int EN_matrix_index = nodeid_map[ln.endNode.node_id];

		// Extract the start and end node displacement
		Eigen::Vector4d element_displacement;

		element_displacement.coeffRef(0) = globalDisplacementMatrix((SN_matrix_index * 2) + 0);
		element_displacement.coeffRef(1) = globalDisplacementMatrix((SN_matrix_index * 2) + 1);
		element_displacement.coeffRef(2) = globalDisplacementMatrix((EN_matrix_index * 2) + 0);
		element_displacement.coeffRef(3) = globalDisplacementMatrix((EN_matrix_index * 2) + 1);

		// Extract the reaction force of start and End node
		Eigen::Vector4d element_resultant;

		element_resultant.coeffRef(0) = globalResultantMatrix((SN_matrix_index * 2) + 0);
		element_resultant.coeffRef(1) = globalResultantMatrix((SN_matrix_index * 2) + 1);
		element_resultant.coeffRef(2) = globalResultantMatrix((EN_matrix_index * 2) + 0);
		element_resultant.coeffRef(3) = globalResultantMatrix((EN_matrix_index * 2) + 1);


		// Transform the Nodal displacement w.r.t support inclination
		// Transformation matrices to include support inclinatation
		Eigen::Matrix4d s_transformation_matrix = Eigen::Matrix4d::Zero(); // support inclination transformation matrix

		int constraint_type;


		// Start node support inclination
		if (cnsts->c_data.find(ln.startNode.node_id) == cnsts->c_data.end())
		{
			// No constraint at the start node
			s_transformation_matrix.row(0) = Eigen::RowVector4d(1.0, 0.0, 0.0, 0.0);
			s_transformation_matrix.row(1) = Eigen::RowVector4d(0.0, 1.0, 0.0, 0.0);
		}
		else
		{
			constraint_type = cnsts->c_data[ln.startNode.node_id].constraint_type; // Constrint type (0 - pin support, 1 - roller support)
			constraint_angle_rad = (cnsts->c_data[ln.startNode.node_id].constraint_angle - 90.0) * (m_pi / 180.0f); // Constrint angle in radians
			support_Lcos = std::cos(constraint_angle_rad); // cosine of support inclination
			support_Msin = std::sin(constraint_angle_rad); // sine of support inclination

			// Pin or Roller Support
			s_transformation_matrix.row(0) = Eigen::RowVector4d(support_Lcos, -support_Msin, 0.0, 0.0);
			s_transformation_matrix.row(1) = Eigen::RowVector4d(support_Msin, support_Lcos, 0.0, 0.0);
		}


		// End node support inclination
		if (cnsts->c_data.find(ln.endNode.node_id) == cnsts->c_data.end())
		{
			// No constraint at the start node
			s_transformation_matrix.row(2) = Eigen::RowVector4d(0.0, 0.0, 1.0, 0.0);
			s_transformation_matrix.row(3) = Eigen::RowVector4d(0.0, 0.0, 0.0, 1.0);
		}
		else
		{
			constraint_type = cnsts->c_data[ln.endNode.node_id].constraint_type; // Constrint type (0 - pin support, 1 - roller support)
			constraint_angle_rad = (cnsts->c_data[ln.endNode.node_id].constraint_angle - 90.0) * (m_pi / 180.0f); // Constrint angle in radians
			support_Lcos = std::cos(constraint_angle_rad); // cosine of support inclination
			support_Msin = std::sin(constraint_angle_rad); // sine of support inclination

			// Pin or Roller Support
			s_transformation_matrix.row(2) = Eigen::RowVector4d(0.0, 0.0, support_Lcos, -support_Msin);
			s_transformation_matrix.row(3) = Eigen::RowVector4d(0.0, 0.0, support_Msin, support_Lcos);
		}

		//__________________________________________________________________________________________________________________
		// Calculate the transformed element displacement
		Eigen::Vector4d element_displacement_transformed;

		element_displacement_transformed = s_transformation_matrix * element_displacement;

		// Calculate the transformed element resultant
		Eigen::Vector4d element_resultant_transformed;

		element_resultant_transformed = s_transformation_matrix * element_resultant;

		//__________________________________________________________________________________________________________________

		// Compute the differences in x and y coordinates
		double dx = ln.endNode.node_pt.x - ln.startNode.node_pt.x;
		double dy = -1.0 * (ln.endNode.node_pt.y - ln.startNode.node_pt.y);

		// Compute the length of the truss element
		double eLength = std::sqrt((dx * dx) + (dy * dy));

		material_data mdata = (*mdatas)[ln.material_id];

		// Get the element properties
		double youngs_mod = mdata.youngs_mod;
		double cs_area = mdata.cs_area;


		// Compute the direction cosines
		double Lcos = (dx / eLength);
		double Msin = (dy / eLength);

		// Calculate member stress, member strain and member force

		double member_strain = ((Lcos * element_displacement_transformed.coeff(0)) +
			(Msin * element_displacement_transformed.coeff(1)) +
			(-Lcos * element_displacement_transformed.coeff(2)) +
			(-Msin * element_displacement_transformed.coeff(3))) / eLength;


		double member_stress = youngs_mod * member_strain;

		double member_force = cs_area * member_stress;

		// Get the material data of this line
		int line_id = ln_m.first;

		// Fix the member Stress and Member Force
		lines->update_results(line_id, member_stress, member_force);

		// Fix the maximum value member stress and member strain
		if (max_memberstress < std::abs(member_stress))
		{
			max_memberstress = std::abs(member_stress);
		}

		if (max_memberforce < std::abs(member_force))
		{
			max_memberforce = std::abs(member_force);
		}
	}




	// Map the results to Nodes and Elements
	double max_displacement = 0.0;
	double max_resultant = 0.0;

	reaction_x.delete_all();
	reaction_y.delete_all();

	for (auto& nd_m : nodes->nodeMap)
	{
		int node_id = nd_m.first;
		int matrix_index = nodeid_map[node_id];

		// Transformation matrices to include support inclinatation
		Eigen::Matrix2d s_transformation_matrix = Eigen::Matrix2d::Zero(); // support inclination transformation matrix

		// Assume No constraint at the node (Free)
		s_transformation_matrix.row(0) = Eigen::RowVector2d(1.0, 0.0);
		s_transformation_matrix.row(1) = Eigen::RowVector2d(0.0, 1.0);


		// Extract the node resultant
		double resultant_x = globalResultantMatrix((matrix_index * 2) + 0);
		double resultant_y = globalResultantMatrix((matrix_index * 2) + 1);

		double constraint_angle = 0.0;

		if (cnsts->c_data.find(node_id) != cnsts->c_data.end())
		{
			// Constraint present in this node
			constraint_angle = cnsts->c_data[node_id].constraint_angle; // Constrint angle in radians

			// Create the reaction force x
			if (std::roundf(static_cast<float>(resultant_x)) != 0)
			{
				double reaction_val_x = resultant_x;
				double constraint_angle_x = constraint_angle - 90.0f;

				// constraint_angle_x = constraint_angle_x > 360.0f ? (constraint_angle_x - 360.0f) : constraint_angle_x;

				if (reaction_val_x < 0)
				{
					reaction_val_x = -1 * reaction_val_x;
					constraint_angle_x = (constraint_angle_x + 180.0f);
				}

				constraint_angle_x = constraint_angle_x > 360.0f ? (constraint_angle_x - 360.0f) : constraint_angle_x;

				// Set the reaction force x
				reaction_x.add_load(node_id, &nd_m.second, reaction_val_x, constraint_angle_x);
			}

			// Create the reaction force y
			if (std::roundf(resultant_y) != 0)
			{
				double reaction_val_y = resultant_y;
				double constraint_angle_y = constraint_angle;

				// constraint_angle_y = constraint_angle_y > 360.0f ? (constraint_angle_y - 360.0f) : constraint_angle_y;

				if (reaction_val_y < 0)
				{
					reaction_val_y = -1 * reaction_val_y;
					constraint_angle_y = constraint_angle_y + 180.0f;
				}

				constraint_angle_y = constraint_angle_y > 360.0f ? (constraint_angle_y - 360.0f) : constraint_angle_y;

				// Set the reaction force y
				reaction_y.add_load(node_id, &nd_m.second, reaction_val_y, constraint_angle_y);
			}


			constraint_angle_rad = (constraint_angle - 90.0) * (m_pi / 180.0f); // Constrint angle in radians
			support_Lcos = std::cos(constraint_angle_rad); // cosine of support inclination
			support_Msin = std::sin(constraint_angle_rad); // sine of support inclination

			// Pin or Roller Support
			//s_transformation_matrix.row(0) = Eigen::RowVector2d(support_Lcos, -support_Msin);
			//s_transformation_matrix.row(1) = Eigen::RowVector2d(support_Msin, support_Lcos);

			if (cnsts->c_data[node_id].constraint_type == 0)
			{
				// Pinned
				// s_transformation_matrix.row(0) = Eigen::RowVector2d(1, -1);
				// s_transformation_matrix.row(1) = Eigen::RowVector2d(-1, 1);

				s_transformation_matrix.row(0) = Eigen::RowVector2d(1.0, 0.0);
				s_transformation_matrix.row(1) = Eigen::RowVector2d(0.0, 1.0);

			}
			else if (cnsts->c_data[node_id].constraint_type == 1)
			{
				// Roller
				//s_transformation_matrix.row(0) = Eigen::RowVector2d(support_Lcos, -support_Msin );
				//s_transformation_matrix.row(1) = Eigen::RowVector2d(-support_Msin, support_Lcos);

				//s_transformation_matrix.row(0) = Eigen::RowVector2d(support_Lcos * support_Lcos, -support_Lcos * support_Msin);
				//s_transformation_matrix.row(1) = Eigen::RowVector2d(-support_Lcos * support_Msin, support_Msin * support_Msin);

				//s_transformation_matrix.row(0) = Eigen::RowVector2d(support_Msin * support_Msin, -support_Lcos * support_Msin);
				//s_transformation_matrix.row(1) = Eigen::RowVector2d(-support_Lcos * support_Msin, support_Lcos * support_Lcos);

				s_transformation_matrix.row(0) = Eigen::RowVector2d(1.0, 0.0);
				s_transformation_matrix.row(1) = Eigen::RowVector2d(0.0, 1.0);

			}



		}


		// Extract the node displacement 
		Eigen::Vector2d node_displacement;
		node_displacement.coeffRef(0) = globalDisplacementMatrix((matrix_index * 2) + 0);
		node_displacement.coeffRef(1) = globalDisplacementMatrix((matrix_index * 2) + 1);


		// Extract the node displacement transformed
		Eigen::Vector2d node_displacement_transformed = s_transformation_matrix * node_displacement;


		double displ_x = node_displacement_transformed[0];
		double displ_y = node_displacement_transformed[1];

		nodes->update_results(node_id, displ_x, displ_y, resultant_x, resultant_y, constraint_angle);

		// Extract the node displacement and find the resultant 
		double displ_xy = std::sqrt(std::pow(displ_x, 2) + std::pow(displ_y, 2));

		if (displ_xy > max_displacement)
		{
			// fix the maximum displacement
			max_displacement = displ_xy;
		}

		// Extract the node resultant
		double resultant_xy = std::sqrt(std::pow(resultant_x, 2) + std::pow(resultant_y, 2));

		if (resultant_xy > max_resultant)
		{
			// fix the maximum resultant
			max_resultant = resultant_xy;
		}

	}

	if (max_displacement == 0 || max_resultant == 0)
	{
		// Results are not valid
		is_map_success = false;
	}
	else
	{
		// results are valid maping success
		is_map_success = true;
	}

	// Set the maximum values
	nodes->set_result_max(max_displacement, max_resultant); // Set the results of maximim displacement and maximum resultant
	lines->set_result_max(max_displacement, max_resultant, max_memberstress, max_memberforce);

	// Create the buffers for reaction force
	reaction_x.set_buffer();
	reaction_y.set_buffer();

	// Set the maximum value for load
	reaction_x.max_load = max_resultant;
	reaction_y.max_load = max_resultant;

	reaction_x.update_buffer();
	reaction_y.update_buffer();

	reaction_x.update_geometry_matrices(true, true, true, true, true);
	reaction_y.update_geometry_matrices(true, true, true, true, true);



}