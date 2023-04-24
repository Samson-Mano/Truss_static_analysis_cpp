#pragma once

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
	void update_event(menu_item m_ck);
private:

};
