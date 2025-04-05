#pragma once
#include <unordered_map>
#include "geom_parameters.h"
#include "nodes_store_list.h"
#include "lines_store_list.h"
#include "shaders/shader.h"
#include <filesystem>
#include "mconstraints.h"
#include "mloads.h"
#include "../options_window.h"
#include "../material_window.h"
#include "../solver_window.h"
#include "solver/fe_solver.h"
#include "solver/penalty_solver.h"
#include "label_text_store.h"
#include <fstream>
#include <sstream>

class geom_store
{
public:
	bool is_geometry_loaded;
	bool is_geometry_set;

	// Main Variable to strore the geometry parameters
	geom_parameters geom_param;

	geom_store();
	~geom_store();
	void init();
	void write_rawdata(std::ofstream& file);
	void read_dxfdata(std::ostringstream& input_data);
	void read_rawdata(std::ifstream& input_file);
	void read_varai2d(std::ifstream& input_file);
	void add_window_ptr(options_window* op_window, material_window* mat_window, solver_window* fe_window);
	void paint_geometry();
	void updateWindowDimension(const int& window_width, const int& window_height);
	void zoomfit_geometry();
	void pan_geometry(glm::vec2& transl);
	void zoom_geometry(float& z_scale);
	void set_nodal_loads(glm::vec2& loc, double& load_value, double& load_angle, bool is_add);
	void set_nodal_constraints(glm::vec2& loc, int& constraint_type, float& constraint_angle, bool is_add);
	void set_line_material(glm::vec2& loc);
	void update_delete_material(int& del_material_id);
private:
	// Geometry objects
	nodes_store_list model_nodes;
	lines_store_list model_lines;
	// Constraint data store
	mconstraints constraintMap;
	// Load data store
	mloads loadMap;
	// Result reaction force
	mloads reaction_x;
	mloads reaction_y;

	// View options ptr and Material window ptr
	options_window* op_window;
	material_window* mat_window;
	solver_window* fe_window;

	// Solver store
	fe_solver fe_sol;
	penalty_solver fe_penalty_sol;

	// Store the openGL buffers
	shader text_shader;
	shader result_text_shader;

	// Functions to set the geometry
	void paint_model();
	void update_model_transperency(bool is_transparent);
	void deleteResources();
	void set_geometry();
	void create_geometry(nodes_store_list& model_nodes,
		lines_store_list& lineMap,
		mconstraints& constraintMap,
		mloads& loadMap);

	std::pair<glm::vec3, glm::vec3> findMinMaxXY(const std::unordered_map<int, nodes_store>& nodeMap);
	glm::vec3 findGeometricCenter(const std::unordered_map<int, nodes_store>& nodeMap);
	void set_model_matrix();
	
};
