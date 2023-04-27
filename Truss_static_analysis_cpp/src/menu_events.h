#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <Windows.h>
#include <Commdlg.h>
#include <limits>
#include "geometry_store/nodes_store.h" // header file for the nodes_store class
#include "geometry_store/lines_store.h" // header file for the lines_store class
#include "geometry_store/geom_store.h" // header file for the geom_store class
#include <sstream>

enum menu_item
{
	import_varai2d,
	import_raw_data,
	export_raw_data,
	exit_p,
	Add_Constraint,
	Delete_Constraint,
	Edit_Constraint,
	Add_Load,
	Delete_Load,
	Edit_Load,
	FE_Solve,
	Displacement,
	Member_force
};

class menu_events
{
public:
	menu_events();
	~menu_events();
	void update_event(menu_item m_ck, geom_store* geom);
private:
	std::string fileNameStr = "";  // Initialize with an empty string
	std::string ShowFileDialog();
	void import_varai2d_geometry(geom_store* geom);
};
