#include <iostream>
#include <windows.h>
#include "src/app_window.h"

int main()
{
	app_window app;

	if (app.is_glwindow_success == false)
	{
		// Window creation failed
		// Display error in the console
		std::cerr << app.log << std::endl;
		std::cin.get();
		return -1;
	}
	else
	{
		// Window creation successful (Hide the console window)
		// ShowWindow(GetConsoleWindow(), SW_HIDE); //SW_RESTORE to bring back
		app.app_render();
	}

	return 0;
}
