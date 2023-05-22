#pragma once
// #include "../geom_store.h"
#include "../../solver_window.h"
#include "../nodes_store_list.h"
#include "../lines_store_list.h"
#include "../mconstraints.h"
#include "../mloads.h"

class fe_solver
{
public:
	fe_solver();
	~fe_solver();
	void solve_start(nodes_store_list* nodes,
		lines_store_list* lines,
		mconstraints* cnsts,
		mloads* loads,
		solver_window* fe_window);

private:
};