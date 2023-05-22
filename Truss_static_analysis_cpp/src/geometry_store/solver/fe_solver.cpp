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
	solver_window* fe_window)
{
	// Main Solver Start
	fe_window->is_analysis_complete = false;

	// Check the model
	fe_window->log_buffer = "Analysis started \n";
	fe_window->log_buffer.append("Checking the model \n");



}
