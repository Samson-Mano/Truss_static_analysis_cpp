#pragma once
#include <iostream>
#include <fstream>
#include "../../solver_window.h"
#include "../nodes_store_list.h"
#include "../lines_store_list.h"
#include "../mconstraints.h"
#include "../mloads.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>



class lagrange_solver
{
public:
	const double m_pi = 3.14159265358979323846;
	bool print_matrix = true;

	lagrange_solver();
	~lagrange_solver();

	void solve_start(nodes_store_list* nodes,
		lines_store_list* lines,
		mconstraints* cnsts,
		mloads* loads,
		std::unordered_map<int, material_data>* mdatas,
		mloads& reaction_x,
		mloads& reaction_y,
		solver_window* fe_window);



private:
	std::unordered_map<int, int> nodeid_map;

	// Penalty stiffness
	double max_stiffness = 0.0;
	const double penalty_factor = 1E+6;


	void get_global_stiffness_matrix(Eigen::MatrixXd& globalStiffnessMatrix, lines_store_list* lines,
		std::unordered_map<int, material_data>* mdatas, mconstraints* cnsts, std::ofstream& output_file);


	void get_element_stiffness_matrix(Eigen::Matrix4d& elementStiffnessMatrix, lines_store& ln,
		material_data& mdata, mconstraints* cnsts, std::ofstream& output_file);


	void get_global_force_vector(Eigen::VectorXd& globalForceVector,
		nodes_store_list* nodes, mloads* loads, std::ofstream& output_file);


	void get_global_constraint_A_matrix(Eigen::MatrixXd& globalConstraint_SPC_AMatrix,
		Eigen::MatrixXd& globalConstraint_MPC_AMatrix, int numDOF,
		nodes_store_list* nodes, lines_store_list* lines, std::unordered_map<int, material_data>* mdatas,
		mconstraints* cnsts, std::ofstream& output_file);


	void map_analysis_results(Eigen::VectorXd& globalDisplacementVector,
		Eigen::VectorXd& globalResultantVector,
		nodes_store_list* nodes,
		lines_store_list* lines,
		mconstraints* cnsts,
		mloads* loads,
		mloads& reaction_x,
		mloads& reaction_y,
		std::unordered_map<int, material_data>* mdatas,
		bool& is_map_success,
		std::ofstream& output_file);



};


