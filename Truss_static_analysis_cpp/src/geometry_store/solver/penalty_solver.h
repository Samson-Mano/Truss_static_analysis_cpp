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


class penalty_solver
{
public:
	const double m_pi = 3.14159265358979323846;
	bool print_matrix = true;

	penalty_solver();
	~penalty_solver();

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

	double penaltyK = 0.0; // Penalty stiffness

	void get_global_stiffness_matrix(Eigen::MatrixXd& globalStiffnessMatrix, lines_store_list* lines,
		std::unordered_map<int, material_data>* mdatas, mconstraints* cnsts, std::ofstream& output_file);


	void get_element_stiffness_matrix(Eigen::Matrix4d& elementStiffnessMatrix, lines_store& ln,
		material_data& mdata, mconstraints* cnsts, std::ofstream& output_file);


	void get_global_force_matrix(Eigen::VectorXd& globalForceMatrix,
		nodes_store_list* nodes, mloads* loads, std::ofstream& output_file);


	void get_boundary_condition_penalty_matrix(Eigen::MatrixXd& globalPenaltyStiffnessMatrix, int numDOF,
		nodes_store_list* nodes, lines_store_list* lines, mconstraints* cnsts, std::ofstream& output_file);


	void map_analysis_results(Eigen::VectorXd& globalDisplacementMatrix,
		Eigen::VectorXd& globalResultantMatrix,
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