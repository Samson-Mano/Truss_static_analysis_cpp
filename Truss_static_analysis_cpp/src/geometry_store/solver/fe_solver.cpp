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
	//____________________________________________________________________________________________________________________
	// Find the maximum displacement and resultant
	// double max_displacement_x = 1.0E-16;
	// double max_displacement_y = 1.0E-16;
	double max_displacement = 0.0;
	double max_resultant = 0.0;
	for (auto& nd : nodes->nodeMap)
	{
		int matrix_index = nodeid_map[nd.first];

		// Extract the node displacement and find the resultant 
		double displ_xy = std::sqrt(std::pow(globalDisplacementMatrix((matrix_index * 2) + 0, 0), 2) + 
			std::pow(globalDisplacementMatrix((matrix_index * 2) + 1, 0), 2));

		//if (std::abs(globalDisplacementMatrix((matrix_index * 2) + 0, 0)) > std::abs(max_displacement_x))
		//{
		//	// Fix the maximum displacement X
		//	max_displacement_x = globalDisplacementMatrix((matrix_index * 2) + 0, 0);
		//}

		//if (std::abs(globalDisplacementMatrix((matrix_index * 2) + 1, 0)) > std::abs(max_displacement_y))
		//{
		//	// Fix the maximum displacement Y
		//	max_displacement_y = globalDisplacementMatrix((matrix_index * 2) + 1, 0);
		//}

		if (displ_xy > max_displacement)
		{
			// fix the maximum displacement
			max_displacement = displ_xy;
		}

		// Extract the node resultant
		double resultant_xy = std::sqrt(std::pow(globalResultantMatrix((matrix_index * 2) + 0, 0), 2) +
			std::pow(globalResultantMatrix((matrix_index * 2) + 1, 0),2));

		if (resultant_xy > max_resultant)
		{
			// fix the maximum resultant
			max_resultant = resultant_xy;
		}
	}

	nodes->set_result_max(max_displacement, max_resultant); // Set the results of maximim displacement and maximum resultant
	lines->set_result_max(max_displacement, max_resultant, 0.0f, 0.0f);

	//____________________________________________________________________________________________________________________
	// Set the analysis

	if (max_displacement == 0 || max_resultant == 0)
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

	//____________________________________________________________________________________________________________________
	// Map the results to Nodes and Elements
	// Map to results to Nodes
	for (auto& nd : nodes->nodeMap)
	{
		int node_id = nd.first;
		int matrix_index = nodeid_map[node_id];

		// Extract the node displacement 
		double displ_x = globalDisplacementMatrix((matrix_index * 2) + 0, 0);
		double displ_y = globalDisplacementMatrix((matrix_index * 2) + 1, 0);

		// Extract the node resultant
		double resultant_x = globalResultantMatrix((matrix_index * 2) + 0, 0);
		double resultant_y = globalResultantMatrix((matrix_index * 2) + 1, 0);

		if (cnsts->c_data.find(node_id) == cnsts->c_data.end())
		{
			// No constraint is in this node
			nodes->update_results(node_id, displ_x, displ_y, 0.0, 0.0);
		}
		else
		{
			// Constraint present in this node
			if (cnsts->c_data[node_id].constraint_type == 0)
			{
				// Pin support
				nodes->update_results(node_id, 0.0, 0.0, resultant_x, resultant_y);
			}
			else
			{
				// Roller support
				nodes->update_results(node_id, displ_x, 0.0, 0.0, resultant_y);
			}
		}
	}


	// Map the results to Elements
	for (auto& ln : lines->lineMap)
	{
		// Get the material data of this line
		int line_id = ln.first;
		material_data mdata = (*mdatas)[ln.second.material_id];

		lines->update_results(line_id, mdata.youngs_mod, mdata.cs_area);
	}


	fe_window->log_buffer.append("Results are mapped to elements \n");

	// Set the node buffers
	nodes->set_defl_buffer();
	nodes->update_geometry_matrices(true, true, true, true);

	// Set the line buffers
	lines->set_defl_buffer();
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



