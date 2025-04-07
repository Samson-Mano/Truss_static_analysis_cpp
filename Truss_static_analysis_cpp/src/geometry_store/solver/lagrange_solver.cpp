#include "lagrange_solver.h"

lagrange_solver::lagrange_solver()
{
	// Empty constructor
}

lagrange_solver::~lagrange_solver()
{
	// Empty destructor
}


void lagrange_solver::solve_start(nodes_store_list* nodes,
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
	output_file.open("fe_matrices_lagrange.txt");

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
	Eigen::VectorXd globalForceVector(numDOF);
	globalForceVector.setZero();

	get_global_force_vector(globalForceVector, nodes, loads, output_file);

	fe_window->log_buffer.append("3. Global Force Matrix created \n");

	//____________________________________________________________________________________________________________________
	// Global Constraint A matrix
	Eigen::MatrixXd globalConstraint_SPC_AMatrix; // resize inside the function
	Eigen::MatrixXd globalConstraint_MPC_AMatrix; // resize inside the function
		
	get_global_constraint_A_matrix(globalConstraint_SPC_AMatrix, globalConstraint_MPC_AMatrix, numDOF, nodes, lines, mdatas, cnsts, output_file);

	Eigen::MatrixXd globalConstraint_AMatrix; // resize inside the function

	// Create the global constraint A matrix
	int row_spc = globalConstraint_SPC_AMatrix.rows();
	int row_mpc = globalConstraint_MPC_AMatrix.rows();

	globalConstraint_AMatrix.conservativeResize(row_spc + row_mpc, numDOF); // where the row size is SPC A matrix + MPC A matrix

	if (row_spc > 0)
	{
		globalConstraint_AMatrix.topRows(row_spc) = globalConstraint_SPC_AMatrix;
	}

	if (row_mpc > 0)
	{
		globalConstraint_AMatrix.bottomRows(row_mpc) = globalConstraint_MPC_AMatrix;
	}


	if (print_matrix == true)
	{
		// Print the Global Constraint A matrix
		output_file << "Global Constraint A Matrix" << std::endl;
		output_file << std::fixed << std::setprecision(6) << globalConstraint_AMatrix << std::endl;  // Set decimal precision to 6 
		output_file << std::endl;
	}


	fe_window->log_buffer.append("4. Global Constraint A Matrix created \n");


	//____________________________________________________________________________________________________________________
	// Lagrange Augmentation of global stiffness matrix with global constratint A matrix
	int SPCconstraintEqnSize = globalConstraint_SPC_AMatrix.rows();
	int MPCconstraintEqnSize = globalConstraint_MPC_AMatrix.rows();

	int constraintEqnSize = SPCconstraintEqnSize + MPCconstraintEqnSize;

	int LagrageAugmentedMatrixSize = numDOF + constraintEqnSize;

	Eigen::MatrixXd globalLagrangeAugmentedStiffnessMatrix(LagrageAugmentedMatrixSize, LagrageAugmentedMatrixSize);
	globalLagrangeAugmentedStiffnessMatrix.setZero();

	// Top-left block: K
	globalLagrangeAugmentedStiffnessMatrix.topLeftCorner(numDOF, numDOF) = globalStiffnessMatrix;

	// Top-right block: A^T
	globalLagrangeAugmentedStiffnessMatrix.topRightCorner(numDOF, constraintEqnSize) = globalConstraint_AMatrix.transpose();

	// Bottom-left block: A
	globalLagrangeAugmentedStiffnessMatrix.bottomLeftCorner(constraintEqnSize, numDOF) = globalConstraint_AMatrix;

	// Bottom-right block: zero matrix (already zero-initialized)


	if (print_matrix == true)
	{
		// Print the Reduced Global Displacement matrix
		output_file << "Global Lagrange Augmented Stiffness Matrix" << std::endl;
		output_file << globalLagrangeAugmentedStiffnessMatrix << std::endl;
		output_file << std::endl;
	}



	// Lagrange Augmentation of global force matrix
	Eigen::VectorXd globalLagrangeAugmentedForceVector(LagrageAugmentedMatrixSize);
	globalLagrangeAugmentedForceVector.setZero();

	// Fill top part with F
	globalLagrangeAugmentedForceVector.topRows(numDOF) = globalForceVector;

	// Fill bottom part with b (b is zero which is already initialized
	

	if (print_matrix == true)
	{
		// Print the Reduced Global Displacement matrix
		output_file << "Global Lagrange Augmented Force Matrix" << std::endl;
		output_file << globalLagrangeAugmentedForceVector << std::endl;
		output_file << std::endl;
	}

	fe_window->log_buffer.append("5. Global Lagrange Augmented stiffness and force matrix created \n");

	//____________________________________________________________________________________________________________________
	// Global Displacement matrix
	Eigen::VectorXd globalLagrangeAugmentedDisplacementVector(LagrageAugmentedMatrixSize);
	globalLagrangeAugmentedDisplacementVector.setZero();

	// Solve using Partial Pivot LU decomposition
	globalLagrangeAugmentedDisplacementVector = globalLagrangeAugmentedStiffnessMatrix.lu().solve(globalLagrangeAugmentedForceVector);


	// Get the global dispalcement and lagrange multipliers
	Eigen::VectorXd globalDisplacementVector(numDOF);
	globalDisplacementVector.setZero();

	Eigen::VectorXd globalLagrangeMultiplierVector(constraintEqnSize);
	globalLagrangeMultiplierVector.setZero();


	// global displacement
	globalDisplacementVector = globalLagrangeAugmentedDisplacementVector.topRows(numDOF);

	// lagrange multiplier (SPC)
	globalLagrangeMultiplierVector = globalLagrangeAugmentedDisplacementVector.segment(numDOF, SPCconstraintEqnSize);


	if (print_matrix == true)
	{
		// Print the Global Lagrange Augmented Displacement matrix
		output_file << "Global Displacement Matrix" << std::endl;
		output_file << globalDisplacementVector << std::endl;
		output_file << std::endl;

		output_file << "Lagrange Multipliers SPC" << std::endl;
		output_file << globalLagrangeMultiplierVector << std::endl;
		output_file << std::endl;
	}

	fe_window->log_buffer.append("6. LU Decomposition Matrix solution for Displacement completed \n");

	//____________________________________________________________________________________________________________________
	// Global Resultant matrix
	Eigen::VectorXd globalResultantVector(numDOF);
	globalResultantVector.setZero();

	globalResultantVector = -1.0 * (globalConstraint_SPC_AMatrix.transpose() * globalLagrangeMultiplierVector);


	if (print_matrix == true)
	{
		// Print the Global Resultant matrix
		output_file << "Global Resultant Matrix" << std::endl;
		output_file << globalResultantVector << std::endl;
		output_file << std::endl;
	}

	fe_window->log_buffer.append("7. Resultant forces calculated \n");


	//____________________________________________________________________________________________________________________
	// Map the analysis Results
	bool is_map_success = false;

	map_analysis_results(globalDisplacementVector,
		globalResultantVector,
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



void lagrange_solver::get_global_stiffness_matrix(Eigen::MatrixXd& globalStiffnessMatrix,
	lines_store_list* lines, std::unordered_map<int, material_data>* mdatas, mconstraints* cnsts, std::ofstream& output_file)
{
	this->max_stiffness = 0.0;

	// Set the maximum stiffnes
	for (auto& ln_m : lines->lineMap)
	{
		lines_store ln = ln_m.second;
		material_data mdata = (*mdatas)[ln.material_id];

		if (mdata.material_id != 0)
		{
			// Compute the differences in x and y coordinates
			double dx = ln.endNode.node_pt.x - ln.startNode.node_pt.x;
			double dy = -1.0 * (ln.endNode.node_pt.y - ln.startNode.node_pt.y);

			// Compute the length of the truss element
			double eLength = std::sqrt((dx * dx) + (dy * dy));

			double ln_stiffness = (mdata.cs_area * mdata.youngs_mod) / eLength;

			// Set the maximum stiffness
			this->max_stiffness = std::max(this->max_stiffness, ln_stiffness);
		}
	}

	if (this->max_stiffness < (1.0 / this->penalty_factor))
	{
		// This case is to handle when all the elements are rigid links
		this->max_stiffness = (1.0 / this->penalty_factor);
	}


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

	if (print_matrix == true)
	{
		// Print the Global Stiffness matrix
		output_file << "Global Stiffness Matrix" << std::endl;
		output_file << globalStiffnessMatrix << std::endl;
		output_file << std::endl;
	}

}



void lagrange_solver::get_element_stiffness_matrix(Eigen::Matrix4d& elementStiffnessMatrix,
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
		k1 = 0;  // this->max_stiffness * this->penalty_factor;

	}
	else
	{
		// Flexible element
		k1 = (mdata.cs_area * mdata.youngs_mod) / eLength;

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



void lagrange_solver::get_global_force_vector(Eigen::VectorXd& globalForceVector,
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

			globalForceVector((nd_map * 2) + 0, 0) += f_x;
			globalForceVector((nd_map * 2) + 1, 0) += f_y;
		}
		else
		{
			// Nodes doesn't have loads
			globalForceVector((nd_map * 2) + 0, 0) += 0.0;
			globalForceVector((nd_map * 2) + 1, 0) += 0.0;
		}
	}

	if (print_matrix == true)
	{
		// Print the Global Force matrix
		output_file << "Global Force Matrix" << std::endl;
		output_file << std::fixed << std::setprecision(6) << globalForceVector << std::endl;  // Set decimal precision to 6 
		output_file << std::endl;
	}

}



void lagrange_solver::get_global_constraint_A_matrix(Eigen::MatrixXd& globalConstraint_SPC_AMatrix, 
	Eigen::MatrixXd& globalConstraint_MPC_AMatrix, int numDOF,
	nodes_store_list* nodes, lines_store_list* lines, std::unordered_map<int, material_data>* mdatas,
	mconstraints* cnsts, std::ofstream& output_file)
{

	// Apply boundary condition using Lagrange method
	// Single point constraint (Pinned or Roller boundary condition)

	globalConstraint_SPC_AMatrix.resize(0, numDOF); // Start with zero rows


	int currentRows = 0;

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

			// Single point constraint A vector
			Eigen::VectorXd SPC_AVector(numDOF);
			SPC_AVector.setZero();


			if (constraint_type == 0)
			{
				// Pinned support
				SPC_AVector[(nd_map * 2) + 0] = 1.0;
				SPC_AVector[(nd_map * 2) + 1] = 0.0;


				// **Expand A_matrix by adding a new row**
				currentRows = globalConstraint_SPC_AMatrix.rows();
				globalConstraint_SPC_AMatrix.conservativeResize(currentRows + 1, numDOF); // Add one row
				globalConstraint_SPC_AMatrix.row(currentRows) = SPC_AVector;       // Insert the new vector (X fixed)

				SPC_AVector[(nd_map * 2) + 0] = 0.0;
				SPC_AVector[(nd_map * 2) + 1] = 1.0;

				// **Expand A_matrix by adding a new row**
				currentRows = globalConstraint_SPC_AMatrix.rows();
				globalConstraint_SPC_AMatrix.conservativeResize(currentRows + 1, numDOF); // Add one row
				globalConstraint_SPC_AMatrix.row(currentRows) = SPC_AVector;       // Insert the new vector (Y fixed)

			}
			else if (constraint_type == 1)
			{
				// Roller support
				SPC_AVector[(nd_map * 2) + 0] = -support_Msin;
				SPC_AVector[(nd_map * 2) + 1] = support_Lcos;

				// **Expand A_matrix by adding a new row**
				currentRows = globalConstraint_SPC_AMatrix.rows();
				globalConstraint_SPC_AMatrix.conservativeResize(currentRows + 1, numDOF); // Add one row
				globalConstraint_SPC_AMatrix.row(currentRows) = SPC_AVector;       // Insert the new vector

			}

		}

	}


	// Multi point constraint (Rigid link)

	globalConstraint_MPC_AMatrix.resize(0, numDOF); // Start with zero rows


	for (auto& ln_m : lines->lineMap)
	{
		// Get the element data
		lines_store ln = ln_m.second;
		material_data mdata = (*mdatas)[ln.material_id];

		if (mdata.material_id == 0)
		{
			// Rigid link

			// Get the Node ID
			int sn_id = nodeid_map[ln.startNode.node_id]; // get the ordered map of the start node ID
			int en_id = nodeid_map[ln.endNode.node_id]; // get the ordered map of the end node ID

			// Compute the differences in x and y coordinates
			double dx = ln.endNode.node_pt.x - ln.startNode.node_pt.x;
			double dy = -1.0 * (ln.endNode.node_pt.y - ln.startNode.node_pt.y);

			// Compute the length of the truss element
			double eLength = std::sqrt((dx * dx) + (dy * dy));

			// Compute the direction cosines
			double Lcos = (dx / eLength);
			double Msin = (dy / eLength);

			// Multi point constraint A vector
			Eigen::VectorXd MPC_AVector(numDOF);
			MPC_AVector.setZero();

			// start node transformation
			MPC_AVector[(sn_id * 2) + 0] = Lcos;
			MPC_AVector[(sn_id * 2) + 1] = Msin;

			// end node transformation
			MPC_AVector[(en_id * 2) + 0] = -Lcos;
			MPC_AVector[(en_id * 2) + 1] = -Msin;

			// **Expand A_matrix by adding a new row**
			currentRows = globalConstraint_MPC_AMatrix.rows();
			globalConstraint_MPC_AMatrix.conservativeResize(currentRows + 1, numDOF); // Add one row
			globalConstraint_MPC_AMatrix.row(currentRows) = MPC_AVector;       // Insert the new vector

		}

	}

}



void lagrange_solver::map_analysis_results(Eigen::VectorXd& globalDisplacementVector,
	Eigen::VectorXd& globalResultantVector,
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

		element_displacement.coeffRef(0) = globalDisplacementVector((SN_matrix_index * 2) + 0);
		element_displacement.coeffRef(1) = globalDisplacementVector((SN_matrix_index * 2) + 1);
		element_displacement.coeffRef(2) = globalDisplacementVector((EN_matrix_index * 2) + 0);
		element_displacement.coeffRef(3) = globalDisplacementVector((EN_matrix_index * 2) + 1);

		// Compute the differences in x and y coordinates
		double dx = ln.endNode.node_pt.x - ln.startNode.node_pt.x;
		double dy = -1.0 * (ln.endNode.node_pt.y - ln.startNode.node_pt.y);

		// Compute the length of the truss element
		double eLength = std::sqrt((dx * dx) + (dy * dy));

		material_data mdata = (*mdatas)[ln.material_id];

		// Get the element properties
		double youngs_mod = 0.0;

		if (mdata.material_id == 0)
		{
			// Rigid element
			youngs_mod = this->max_stiffness * (eLength / mdata.cs_area) * this->penalty_factor;

		}
		else
		{
			// Flexible factor
			youngs_mod = mdata.youngs_mod;

		}

		double cs_area = mdata.cs_area;

		// Compute the direction cosines
		double Lcos = (dx / eLength);
		double Msin = (dy / eLength);

		// Calculate member stress, member strain and member force

		double member_strain = ((Lcos * element_displacement.coeff(0)) +
			(Msin * element_displacement.coeff(1)) +
			(-Lcos * element_displacement.coeff(2)) +
			(-Msin * element_displacement.coeff(3))) / eLength;


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


		// Extract the node resultant
		double resultant_x = 0.0; // globalResultantVector((matrix_index * 2) + 0);
		double resultant_y = 0.0; // globalResultantVector((matrix_index * 2) + 1);

		double constraint_angle = 0.0;

		if (cnsts->c_data.find(node_id) != cnsts->c_data.end())
		{
			// Constraint present in this node
			constraint_angle = cnsts->c_data[node_id].constraint_angle; // Constrint angle in radians

			Eigen::Vector2d resultant_vector;
			resultant_vector.coeffRef(0) = globalResultantVector((matrix_index * 2) + 0);
			resultant_vector.coeffRef(1) = globalResultantVector((matrix_index * 2) + 1);

			// Find the support transformation
			double	constraint_angle_rad = (constraint_angle - 90.0) * (m_pi / 180.0f); // Constrint angle in radians
			double	support_Lcos = std::cos(constraint_angle_rad); // cosine of support inclination
			double	support_Msin = std::sin(constraint_angle_rad); // sine of support inclination

			Eigen::Matrix2d s_transformation_matrix;
			s_transformation_matrix.row(0) = Eigen::RowVector2d(support_Lcos, support_Msin);
			s_transformation_matrix.row(1) = Eigen::RowVector2d(-support_Msin, support_Lcos);

			// Extract the resultant vector transformed
			Eigen::Vector2d resultant_vector_transformed = s_transformation_matrix * resultant_vector;

			resultant_x = resultant_vector_transformed[0];
			resultant_y = resultant_vector_transformed[1];


			// Create the reaction force x
			if (std::roundf(static_cast<float>(resultant_x)) != 0)
			{
				double reaction_val_x = resultant_x;
				double constraint_angle_x = constraint_angle - 90.0f;

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

				if (reaction_val_y < 0)
				{
					reaction_val_y = -1 * reaction_val_y;
					constraint_angle_y = constraint_angle_y + 180.0f;
				}

				constraint_angle_y = constraint_angle_y > 360.0f ? (constraint_angle_y - 360.0f) : constraint_angle_y;

				// Set the reaction force y
				reaction_y.add_load(node_id, &nd_m.second, reaction_val_y, constraint_angle_y);
			}

		}


		double displ_x = globalDisplacementVector((matrix_index * 2) + 0);
		double displ_y = globalDisplacementVector((matrix_index * 2) + 1);

		nodes->update_results(node_id, displ_x, displ_y, 0.0, 0.0, 0.0);

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


