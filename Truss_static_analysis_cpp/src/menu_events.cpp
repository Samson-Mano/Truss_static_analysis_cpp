#include "menu_events.h"
#include <Windows.h>
#include <Commdlg.h>
#include <string>

menu_events::menu_events()
{
}

menu_events::~menu_events()
{
}

void menu_events::update_event(menu_item m_ck)
{
	// Menu item m is clicked
	switch (m_ck)
	{
	case import_varai2d:
		fileNameStr = ShowFileDialog();
		std::cout << "Selected File: " << fileNameStr << std::endl;
		break;
	case import_raw_data:
		break;
	case export_raw_data:
		break;
	case exit_p:
		break;
	case Add_Constraint:
		break;
	case Delete_Constraint:
		break;
	case Edit_Constraint:
		break;
	case Add_Load:
		break;
	case Delete_Load:
		break;
	case Edit_Load:
		break;
	case FE_Solve:
		break;
	case Displacement:
		break;
	case Member_force:
		break;
	default:
		break;
	}

}


// Function to show a file dialog and return the selected file name as std::string
std::string menu_events::ShowFileDialog()
{
	OPENFILENAMEW ofn;                         // Structure to store the file dialog options (wide-character version)
	wchar_t fileName[MAX_PATH];                // Buffer to store the selected file path (wide-character version)

	// Initialize the OPENFILENAMEW structure
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = fileName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Show the file dialog
	if (GetOpenFileNameW(&ofn))             // Note the 'W' suffix for wide-character version of the function
	{
		// Convert the wide-character string to narrow-character string (UTF-8)
		int bufferSize = WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, nullptr, 0, nullptr, nullptr);
		std::string fileName(bufferSize, '\0');
		WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, &fileName[0], bufferSize, nullptr, nullptr);
		return fileName;
	}

	return "";  // Return an empty string if the file dialog was cancelled or an error occurred
}