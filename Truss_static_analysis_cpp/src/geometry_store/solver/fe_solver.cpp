#include "fe_solver.h"

fe_solver::fe_solver()
{
}

fe_solver::~fe_solver()
{
}

void fe_solver::solve_start(nodes_store_list* nodes,
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
	output_file.open("fe_matrices.txt");

	//____________________________________________________________________________________________________________________
	int numDOF = nodes->node_count * 2; // Number of degrees of freedom (2 DOFs per node)

	// Global Stiffness Matrix
	Eigen::MatrixXd globalStiffnessMatrix(numDOF, numDOF);
	globalStiffnessMatrix.setZero();


	get_global_stiffness_matrix(globalStiffnessMatrix, lines, mdatas, cnsts, output_file);

	fe_window->log_buffer.append("2. Global Stiffness Matrix created \n");
	//____________________________________________________________________________________________________________________
	// Global force matrix
	Eigen::MatrixXd globalForceMatrix(numDOF, 1);
	globalForceMatrix.setZero();

	get_global_force_matrix(globalForceMatrix, nodes, loads, output_file);

	fe_window->log_buffer.append("3. Global Force Matrix created \n");
	//____________________________________________________________________________________________________________________
	// Define a vector to store the indices of unconstrained degrees of freedom
	std::unordered_map<int, int> dofIndices;

	// Determine the size of the reduced stiffness matrix based on the number of unconstrained degrees of freedom
	int reducedDOF = 0;

	get_global_dof_matrix(dofIndices, reducedDOF, nodes, cnsts, output_file);

	//____________________________________________________________________________________________________________________
	// Reduced Global force and stiffness matrix
	Eigen::SparseMatrix<double>  reduced_globalStiffnessMatrix(reducedDOF, reducedDOF);
	reduced_globalStiffnessMatrix.setZero();

	// Reduced Global force matrix
	Eigen::SparseMatrix<double>  reduced_globalForceMatrix(reducedDOF, 1);
	reduced_globalForceMatrix.setZero();


	get_reduced_global_matrices(globalStiffnessMatrix, globalForceMatrix,
		reduced_globalStiffnessMatrix, reduced_globalForceMatrix, dofIndices, output_file);

	fe_window->log_buffer.append("4. Global Matrices reduced based on Constraints \n");

	//____________________________________________________________________________________________________________________
	// Reduced Displacement matrix
	Eigen::SparseMatrix<double> reduced_globalDisplacementMatrix(reducedDOF, 1);
	reduced_globalDisplacementMatrix.setZero();

	//____________________________________________________________________________________________________________________
	// Main solve using Sparse Matric Solver
	Eigen::ConjugateGradient<Eigen::SparseMatrix<double>> solver;
	solver.compute(reduced_globalStiffnessMatrix);
	reduced_globalDisplacementMatrix = solver.solve(reduced_globalForceMatrix);

	if (print_matrix == true)
	{
		// Print the Reduced Global Displacement matrix
		output_file << "Reduced Global Displacement Matrix" << std::endl;
		output_file << reduced_globalDisplacementMatrix << std::endl;
		output_file << std::endl;
	}

	//____________________________________________________________________________________________________________________
	// Global Displacement matrix
	Eigen::MatrixXd globalDisplacementMatrix(numDOF, 1);
	globalDisplacementMatrix.setZero();

	get_global_displacement_matrix(globalDisplacementMatrix, reduced_globalDisplacementMatrix, dofIndices, output_file);

	fe_window->log_buffer.append("5. Sparse Matrix solution for Displacement completed \n");

	//____________________________________________________________________________________________________________________
	// Global Resultant matrix
	Eigen::MatrixXd globalResultantMatrix(numDOF, 1);
	globalResultantMatrix.setZero();

	globalResultantMatrix = (globalStiffnessMatrix * globalDisplacementMatrix) - globalForceMatrix;

	if (print_matrix == true)
	{
		// Print the Global Resultant matrix
		output_file << "Global Resultant Matrix" << std::endl;
		output_file << globalResultantMatrix << std::endl;
		output_file << std::endl;
	}

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
		dofIndices,
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


	fe_window->log_buffer.append("6. Resultant forces calculated \n");
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

void fe_solver::get_global_stiffness_matrix(Eigen::MatrixXd& globalStiffnessMatrix, lines_store_list* lines, std::unordered_map<int, material_data>* mdatas,
	mconstraints* cnsts, std::ofstream& output_file)
{
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

void fe_solver::get_element_stiffness_matrix(Eigen::Matrix4d& elementStiffnessMatrix, lines_store& ln, material_data& mdata,
	mconstraints* cnsts, std::ofstream& output_file)
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
	double k1 = (mdata.cs_area * mdata.youngs_mod) / eLength;

	//Stiffness matrix components
	double v1 = k1 * std::pow(Lcos, 2);
	double v2 = k1 * std::pow(Msin, 2);
	double v3 = k1 * (Lcos * Msin);

	// Create the Element stiffness matrix
	Eigen::Matrix4d e_stiffness_matrix = Eigen::Matrix4d::Zero();

	e_stiffness_matrix.row(0) = Eigen::RowVector4d(v1, v3, -v1, -v3);
	e_stiffness_matrix.row(1) = Eigen::RowVector4d(v3, v2, -v3, -v2);
	e_stiffness_matrix.row(2) = Eigen::RowVector4d(-v1, -v3, v1, v3);
	e_stiffness_matrix.row(3) = Eigen::RowVector4d(-v3, -v2, v3, v2);


	// Transformation matrices to include support inclinatation
	Eigen::Matrix4d s_transformation_matrix = Eigen::Matrix4d::Zero(); // support inclination transformation matrix
	Eigen::Matrix4d s_transformation_matrix_Transpose = Eigen::Matrix4d::Zero(); // support inclination transformation matrix

	int constraint_type;
	double constraint_angle_rad;
	double support_Lcos;
	double support_Msin;

	// Start node support inclination
	if (cnsts->c_data.find(ln.startNode.node_id) == cnsts->c_data.end())
	{
		// No constraint at the start node
		s_transformation_matrix.row(0) = Eigen::RowVector4d(1.0, 0.0, 0.0, 0.0);
		s_transformation_matrix.row(1) = Eigen::RowVector4d(0.0, 1.0, 0.0, 0.0);

		s_transformation_matrix_Transpose.row(0) = Eigen::RowVector4d(1.0, 0.0, 0.0, 0.0);
		s_transformation_matrix_Transpose.row(1) = Eigen::RowVector4d(0.0, 1.0, 0.0, 0.0);
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

		s_transformation_matrix_Transpose.row(0) = Eigen::RowVector4d(support_Lcos, support_Msin, 0.0, 0.0);
		s_transformation_matrix_Transpose.row(1) = Eigen::RowVector4d(-support_Msin, support_Lcos, 0.0, 0.0);
	}


	// End node support inclination
	if (cnsts->c_data.find(ln.endNode.node_id) == cnsts->c_data.end())
	{
		// No constraint at the start node
		s_transformation_matrix.row(2) = Eigen::RowVector4d(0.0, 0.0, 1.0, 0.0);
		s_transformation_matrix.row(3) = Eigen::RowVector4d(0.0, 0.0, 0.0, 1.0);

		s_transformation_matrix_Transpose.row(2) = Eigen::RowVector4d(0.0, 0.0, 1.0, 0.0);
		s_transformation_matrix_Transpose.row(3) = Eigen::RowVector4d(0.0, 0.0, 0.0, 1.0);
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

		s_transformation_matrix_Transpose.row(2) = Eigen::RowVector4d(0.0, 0.0, support_Lcos, support_Msin);
		s_transformation_matrix_Transpose.row(3) = Eigen::RowVector4d(0.0, 0.0, -support_Msin, support_Lcos);
	}


	// Multiply the matrices
	elementStiffnessMatrix = s_transformation_matrix_Transpose * e_stiffness_matrix * s_transformation_matrix;

	if (print_matrix == true)
	{
		// Print the Element Stiffness matrix
		output_file << "Member (" << ln.startNode.node_id << " -> " << ln.endNode.node_id << ")" << std::endl;
		output_file << e_stiffness_matrix << std::endl;
		output_file << std::endl;
	}
}

void fe_solver::get_global_force_matrix(Eigen::MatrixXd& globalForceMatrix, nodes_store_list* nodes,
	mloads* loads, std::ofstream& output_file)
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
		output_file << globalForceMatrix << std::endl;
		output_file << std::endl;
	}
}

void fe_solver::get_global_dof_matrix(std::unordered_map<int, int>& dofIndices, int& reducedDOF, nodes_store_list* nodes,
	mconstraints* cnsts, std::ofstream& output_file)
{
	// Get the Degree of Freedom matrix
	// Loop through the nodes to identify the unconstrained degrees of freedom
	for (const auto& nd_l : nodes->nodeMap)
	{
		// Get the node data
		nodes_store nd = nd_l.second;
		int nd_map = nodeid_map[nd.node_id]; // get the ordered map of the node ID

		if (cnsts->c_data.find(nd.node_id) == cnsts->c_data.end())
		{
			// Node does not have a constraint
			dofIndices[((nd_map * 2) + 0)] = 1; // x
			dofIndices[((nd_map * 2) + 1)] = 1; // y
			reducedDOF = reducedDOF + 2;
		}
		else
		{
			if (cnsts->c_data[nd.node_id].constraint_type == 0)
			{
				// Pin support
				dofIndices[((nd_map * 2) + 0)] = 0; // x
				dofIndices[((nd_map * 2) + 1)] = 0; // y
			}
			else if (cnsts->c_data[nd.node_id].constraint_type == 1)
			{
				// Roller support
				dofIndices[((nd_map * 2) + 0)] = 1; // x
				dofIndices[((nd_map * 2) + 1)] = 0; // y
				reducedDOF = reducedDOF + 1; // 1 DOF is free to move
			}
		}
	}


	if (print_matrix == true)
	{
		// Print the Global DOF matrix
		//output_file << "Global DOF Matrix" << std::endl;
		//output_file << dofIndices << std::endl;
		//output_file << std::endl;
	}
}


void fe_solver::get_reduced_global_matrices(Eigen::MatrixXd& globalStiffnessMatrix, Eigen::MatrixXd& globalForceMatrix,
	Eigen::SparseMatrix<double>& reduced_globalStiffnessMatrix, Eigen::SparseMatrix<double>& reduced_globalForceMatrix,
	std::unordered_map<int, int>& dofIndices, std::ofstream& output_file)
{
	// Curtailment of Global stiffness and Global force matrix based on DOF
	// Get the reduced global stiffness matrix
	int r = 0;
	int s = 0;

	// Loop throug the Degree of freedom of indices
	int numDOF = dofIndices.size();


	for (int i = 0; i < numDOF; i++)
	{
		if (dofIndices[i] == 0)
		{
			// constrained row index, so skip
			continue;
		}
		else
		{
			s = 0;
			for (int j = 0; j < numDOF; j++)
			{
				if (dofIndices[j] == 0)
				{
					// constrained column index, so skip
					continue;
				}
				else
				{
					// Get the reduced matrices
					reduced_globalForceMatrix.coeffRef(s, 0) = globalForceMatrix.coeffRef(j, 0);
					reduced_globalStiffnessMatrix.coeffRef(r, s) = globalStiffnessMatrix.coeffRef(i, j);
					s++;
				}
			}
			r++;
		}
	}

	if (print_matrix == true)
	{
		// Print the Reduced Global Stiffness and Reduced Global Force matrix
		output_file << "Reduced Global Stiffness Matrix" << std::endl;
		output_file << reduced_globalStiffnessMatrix << std::endl;
		output_file << std::endl;

		output_file << "Reduced Global Force Matrix" << std::endl;
		output_file << reduced_globalForceMatrix << std::endl;
		output_file << std::endl;
	}
}

void fe_solver::get_global_displacement_matrix(Eigen::MatrixXd& globalDisplacementMatrix,
	Eigen::SparseMatrix<double>& reduced_globalDisplacementMatrix,
	std::unordered_map<int, int>& dofIndices, std::ofstream& output_file)
{
	// Global Displacement Matrix

	// Loop throug the Degree of freedom of indices
	int numDOF = dofIndices.size();

	int s = 0;
	for (int i = 0; i < numDOF; i++)
	{
		if (dofIndices[i] == 0)
		{
			// constrained row index, so Displacement is Zero
			globalDisplacementMatrix.coeffRef(i, 0) = 0;
		}
		else
		{
			// Un constrained row index, so Displacement is Zero
			globalDisplacementMatrix.coeffRef(i, 0) = reduced_globalDisplacementMatrix.coeffRef(s, 0);
			s++;
		}
	}


	if (print_matrix == true)
	{
		// Print the Global Displacement matrix
		output_file << "Global Displacement Matrix" << std::endl;
		output_file << globalDisplacementMatrix << std::endl;
		output_file << std::endl;
	}
}

void fe_solver::map_analysis_results(Eigen::MatrixXd& globalDisplacementMatrix,
	Eigen::MatrixXd globalResultantMatrix,
	nodes_store_list* nodes,
	lines_store_list* lines,
	mconstraints* cnsts,
	mloads* loads,
	mloads& reaction_x,
	mloads& reaction_y,
	std::unordered_map<int, material_data>* mdatas,
	std::unordered_map<int, int>& dofIndices,
	bool& is_map_success, std::ofstream& output_file)
{
	float max_memberforce = 0.0;
	float max_memberstress = 0.0;

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
		Eigen::Matrix<double, 4, 1> element_displacement;

		element_displacement.coeffRef(0, 0) = globalDisplacementMatrix((SN_matrix_index * 2) + 0, 0);
		element_displacement.coeffRef(1, 0) = globalDisplacementMatrix((SN_matrix_index * 2) + 1, 0);
		element_displacement.coeffRef(2, 0) = globalDisplacementMatrix((EN_matrix_index * 2) + 0, 0);
		element_displacement.coeffRef(3, 0) = globalDisplacementMatrix((EN_matrix_index * 2) + 1, 0);

		// Extract the reaction force of start and End node
		Eigen::Matrix<double, 4, 1> element_resultant;

		element_resultant.coeffRef(0, 0) = globalResultantMatrix((SN_matrix_index * 2) + 0, 0);
		element_resultant.coeffRef(1, 0) = globalResultantMatrix((SN_matrix_index * 2) + 1, 0);
		element_resultant.coeffRef(2, 0) = globalResultantMatrix((EN_matrix_index * 2) + 0, 0);
		element_resultant.coeffRef(3, 0) = globalResultantMatrix((EN_matrix_index * 2) + 1, 0);

		// Extract the element Degree of freedom matrix
		Eigen::Matrix<double, 4, 1> element_dof;

		element_dof.coeffRef(0, 0) = dofIndices[((SN_matrix_index * 2) + 0)];
		element_dof.coeffRef(1, 0) = dofIndices[((SN_matrix_index * 2) + 1)];
		element_dof.coeffRef(2, 0) = dofIndices[((EN_matrix_index * 2) + 0)];
		element_dof.coeffRef(3, 0) = dofIndices[((EN_matrix_index * 2) + 1)];

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
		Eigen::Matrix<double, 4, 1> element_displacement_transformed;

		element_displacement_transformed = s_transformation_matrix * element_displacement;

		// Calculate the transformed element resultant
		Eigen::Matrix<double, 4, 1> element_resultant_transformed;

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

	// Create a global support inclination matrix
	int numDOF = nodes->node_count * 2; // Number of degrees of freedom (2 DOFs per node)
	int node_id = 0;

	Eigen::MatrixXd globalSupportInclinationMatrix(numDOF, numDOF);
	globalSupportInclinationMatrix.setZero();

	// Global displacement and resultant matrix transformed
	Eigen::MatrixXd global_displacement_transformed(numDOF, 1);
	global_displacement_transformed.setZero();

	Eigen::MatrixXd global_resultant_transformed(numDOF, 1);
	global_resultant_transformed.setZero();

	// Transform the Nodal results with support inclination
	for (auto& nd_m : nodes->nodeMap)
	{
		node_id = nd_m.first;
		int matrix_index = nodeid_map[node_id];


		if (cnsts->c_data.find(node_id) == cnsts->c_data.end())
		{
			// No constraint is in this node
			globalSupportInclinationMatrix.coeffRef((matrix_index * 2) + 0, (matrix_index * 2) + 0) = 1.0;
			globalSupportInclinationMatrix.coeffRef((matrix_index * 2) + 0, (matrix_index * 2) + 1) = 0.0;

			globalSupportInclinationMatrix.coeffRef((matrix_index * 2) + 1, (matrix_index * 2) + 0) = 0.0;
			globalSupportInclinationMatrix.coeffRef((matrix_index * 2) + 1, (matrix_index * 2) + 1) = 1.0;
		}
		else
		{
			// Constraint present in this node
			constraint_angle_rad = (cnsts->c_data[node_id].constraint_angle - 90.0f) * (m_pi / 180.0f); // Constrint angle in radians
			support_Lcos = std::cos(constraint_angle_rad); // cosine of support inclination
			support_Msin = std::sin(constraint_angle_rad); // sine of support inclination

			// Pin or Roller Support
			globalSupportInclinationMatrix.coeffRef((matrix_index * 2) + 0, (matrix_index * 2) + 0) = support_Lcos;
			globalSupportInclinationMatrix.coeffRef((matrix_index * 2) + 0, (matrix_index * 2) + 1) = -1.0 * support_Msin;

			globalSupportInclinationMatrix.coeffRef((matrix_index * 2) + 1, (matrix_index * 2) + 0) = support_Msin;
			globalSupportInclinationMatrix.coeffRef((matrix_index * 2) + 1, (matrix_index * 2) + 1) = support_Lcos;
		}
	}

	// Transform the global displacement w.r.t support inclination
	global_displacement_transformed = globalSupportInclinationMatrix * globalDisplacementMatrix;

	// Transformt the global resultant w.r.t support inclination
	global_resultant_transformed = globalSupportInclinationMatrix * globalResultantMatrix;


	if (print_matrix == true)
	{
		// Print the Global Displacement Transformed matrix
		output_file << "Global Displacement Matrix - Settlement Transformed" << std::endl;
		output_file << global_displacement_transformed << std::endl;
		output_file << std::endl;

		// Print the Global Resultant Transformed matrix
		output_file << "Global Resultant Matrix - Settlement Transformed" << std::endl;
		output_file << global_resultant_transformed << std::endl;
		output_file << std::endl;
	}


	// Map the results to Nodes and Elements
	float max_displacement = 0.0;
	float max_resultant = 0.0;

	reaction_x.delete_all();
	reaction_y.delete_all();

	for (auto& nd_m : nodes->nodeMap)
	{
		node_id = nd_m.first;
		int matrix_index = nodeid_map[node_id];

		// Extract the node displacement 
		float displ_x = static_cast<float>(global_displacement_transformed((matrix_index * 2) + 0, 0));
		float displ_y = static_cast<float>(global_displacement_transformed((matrix_index * 2) + 1, 0));

		// Extract the node resultant
		float resultant_x = static_cast<float>(globalResultantMatrix((matrix_index * 2) + 0, 0));
		float resultant_y = static_cast<float>(globalResultantMatrix((matrix_index * 2) + 1, 0));

		float constraint_angle = 0.0;

		if (cnsts->c_data.find(node_id) != cnsts->c_data.end())
		{
			// Constraint present in this node
			constraint_angle = cnsts->c_data[node_id].constraint_angle; // Constrint angle in radians

			// Create the reaction force x
			if (std::roundf(static_cast<float>(resultant_x)) != 0)
			{
				float reaction_val_x = resultant_x;
				float constraint_angle_x = constraint_angle - 90.0f;

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
				float reaction_val_y = resultant_y;
				float constraint_angle_y = constraint_angle;

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
		}

		nodes->update_results(node_id, displ_x, displ_y, resultant_x, resultant_y, constraint_angle);

		// Extract the node displacement and find the resultant 
		float displ_xy = std::sqrt(std::pow(displ_x, 2) + std::pow(displ_y, 2));

		if (displ_xy > max_displacement)
		{
			// fix the maximum displacement
			max_displacement = displ_xy;
		}

		// Extract the node resultant
		float resultant_xy = std::sqrt(std::pow(resultant_x, 2) + std::pow(resultant_y, 2));

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

	reaction_x.update_geometry_matrices(true, true, true, true,true);
	reaction_y.update_geometry_matrices(true, true, true, true,true);
}




