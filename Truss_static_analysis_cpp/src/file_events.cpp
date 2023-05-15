#include "file_events.h"


file_events::file_events()
{
	// Empty Constructor
}

file_events::~file_events()
{
	// Destructor
}

void file_events::update_event(menu_item m_ck, geom_store& geom)
{
	// Menu item m is clicked
	switch (m_ck)
	{
	case import_varai2d:
		// Import Varai 2D geometry
		import_varai2d_geometry(geom);
		break;
	case import_raw_data:
		// Import App raw data geometry
		import_rawdata_geometry(geom);
		break;
	case export_raw_data:
		// Export App raw data geometry
		export_rawdata_geometry(geom);
		break;
	case exit_p:
		break;
	default:
		break;
	}

}


// Function to show a file dialog and return the selected file name as std::string
std::string file_events::ShowOpenFileDialog()
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

std::string file_events::ShowSaveFileDialog()
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
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = L"txt";  // Add this line to set the default file extension

	// Show the file dialog
	if (GetSaveFileNameW(&ofn))             // Note the 'W' suffix for wide-character version of the function
	{
		// Convert the wide-character string to narrow-character string (UTF-8)
		int bufferSize = WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, nullptr, 0, nullptr, nullptr);
		std::string fileName(bufferSize, '\0');
		WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, &fileName[0], bufferSize, nullptr, nullptr);
		return fileName;
	}

	return "";  // Return an empty string if the file dialog was cancelled or an error occurred
}

void file_events::import_varai2d_geometry(geom_store& geom)
{
	std::string file_path = ShowOpenFileDialog();
	std::cout << "Selected File: " << file_path << std::endl;

	// Open the input file
	std::ifstream input_file(file_path, std::ifstream::in);

	if (!input_file.is_open()) {
		std::cerr << "Failed to open input file: " << file_path << std::endl;
		return;
	}

	geom.read_varai2d(input_file);

	input_file.close();
}


void file_events::export_rawdata_geometry(geom_store& geom)
{
	/*if (geom.is_geometry_set == false)
		return;*/

	// Export the raw data in native format
	std::string file_path = ShowSaveFileDialog();
	std::cout << "Selected File: " << file_path << std::endl;


	// Open the file for writing
	std::ofstream output_file(file_path);

	// Check if the file was opened successfully
	if (!output_file.is_open())
	{
		std::cout << "Failed to open file for writing!" << std::endl;
		return;
	}

	// Write the model as raw data
	geom.write_rawdata(output_file);

	// Close the file
	output_file.close();
}


void file_events::import_rawdata_geometry(geom_store& geom)
{
	std::string file_path = ShowOpenFileDialog();
	std::cout << "Selected File: " << file_path << std::endl;

	// Open the input file
	std::ifstream input_file(file_path, std::ifstream::in);

	if (!input_file.is_open()) {
		std::cerr << "Failed to open input file: " << file_path << std::endl;
		return;
	}

	geom.read_rawdata(input_file);

	input_file.close();
}