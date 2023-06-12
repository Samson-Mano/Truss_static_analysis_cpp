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
	case import_dxf_data:
		import_dxfdata_geometry(geom);
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

std::string file_events::ShowOpenFileDialog_dxf()
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
	ofn.lpstrFilter = L"AutoCAD R12 LT2 (*.dxf)\0*.dxf\0All Files (*.*)\0*.*\0";
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



void file_events::import_dxfdata_geometry(geom_store& geom)
{
	std::string file_path = ShowOpenFileDialog_dxf();
	std::cout << "Selected File: " << file_path << std::endl;


	// Open the input file
	std::ifstream input_file(file_path, std::ifstream::in);

	// reads R12 LT AutoCAD Dxf file
	// Read the varai2D
	// Read the entire file into a string
	std::string file_contents((std::istreambuf_iterator<char>(input_file)),
		std::istreambuf_iterator<char>());

	// Split the string into lines
	std::istringstream iss(file_contents);
	std::string line;
	std::vector<std::string> lines;
	while (std::getline(iss, line))
	{
		lines.push_back(line);
	}

	// Data type to store the Lines and Points
	struct dxf_points
	{
		int pt_id = 0;
		double pt_x = 0.0;
		double pt_y = 0.0;

		static int is_exist(std::vector<dxf_points>& all_pts, dxf_points& this_pt)
		{
			for (auto& pt : all_pts)
			{
				if (std::abs(pt.pt_x - this_pt.pt_x) < 1e-6 &&
					std::abs(pt.pt_y - this_pt.pt_y) < 1e-6)
				{
					return pt.pt_id;
				}
			}
			return -1;
		}
	};

	struct dxf_lines
	{
		int startpt_id = 0;
		int endpt_id = 0;

		static bool is_exist(std::vector<dxf_lines>& all_lines, dxf_lines& this_ln)
		{
			for (auto& ln : all_lines)
			{
				if (ln.startpt_id == this_ln.startpt_id && ln.endpt_id == this_ln.endpt_id)
				{
					return true;
				}

				if (ln.startpt_id == this_ln.endpt_id && ln.endpt_id == this_ln.startpt_id)
				{
					return true;
				}
			}
			return false;
		}
	};

	std::vector<dxf_points> all_dxf_points;
	std::vector<dxf_lines> all_dxf_lines;

	int j = 0;

	// Process the lines
	while (j < lines.size())
	{
		// Read the LINE Data
		if (lines[j] == "LINE")
		{
			dxf_points ln_start_pt; // line start pt
			dxf_points ln_end_pt; // line end pt

			j = j + 5; // Move to the next five line
			while (lines[j] != "  0")
			{
				if (lines[j] == " 10")
				{
					// Process the x-coordinate (start point)
					ln_start_pt.pt_x = std::stod(lines[j + 1]);
				}
				else if (lines[j] == " 20")
				{
					// Process the y-coordinate (start point)
					ln_start_pt.pt_y = std::stod(lines[j + 1]);
				}
				else if (lines[j] == " 11")
				{
					// Process the x-coordinate (end point)
					ln_end_pt.pt_x = std::stod(lines[j + 1]);
				}
				else if (lines[j] == " 21")
				{
					// Process the y-coordinate (end point)
					ln_end_pt.pt_y = std::stod(lines[j + 1]);
				}

				j++; // Move to the next two line
			}

			// Add to point list
			// Start Point
			int start_ptid = dxf_points::is_exist(all_dxf_points, ln_start_pt);
			if (start_ptid != -1)
			{
				// Point already exists
				// No need to add

			}
			else
			{
				// Point does not exists (Add start point)
				start_ptid = static_cast<unsigned int>(all_dxf_points.size());
				ln_start_pt.pt_id = start_ptid;
				all_dxf_points.push_back(ln_start_pt);
			}

			// End Point
			int end_ptid = dxf_points::is_exist(all_dxf_points, ln_end_pt);
			if (end_ptid != -1)
			{
				// Point already exists
				// No need to add

			}
			else
			{
				// Point does not exists (Add End point)
				end_ptid = static_cast<unsigned int>(all_dxf_points.size());
				ln_end_pt.pt_id = end_ptid;
				all_dxf_points.push_back(ln_end_pt);
			}

			// Add line
			if (start_ptid != end_ptid)
			{
				dxf_lines line_segment;
				line_segment.startpt_id = start_ptid;
				line_segment.endpt_id = end_ptid;

				// Add line to the list
				if (dxf_lines::is_exist(all_dxf_lines, line_segment) == false)
				{
					all_dxf_lines.push_back(line_segment);
				}
			}
		}

		// Read the POLYLINE Data
		if (lines[j] == "POLYLINE")
		{
			std::vector<dxf_points> polyline_points; // polyline point
			dxf_points pln_pt;
			bool first_pt = true;

			j++; // Move to the next line
			while (lines[j] != "SEQEND")
			{
				if (lines[j] == " 10")
				{
					// Process the polyline x-coordinate
					pln_pt.pt_x = std::stod(lines[j + 1]);

				}
				else if (lines[j] == " 20")
				{
					// Process the polyline y-coordinate
					pln_pt.pt_y = std::stod(lines[j + 1]);

					if (first_pt == false)
					{
						// Add to polyline points list
						if (dxf_points::is_exist(polyline_points, pln_pt) == -1)
						{
							// No points exists so add to the list
							polyline_points.push_back(pln_pt);
						}
					}
					else
					{
						first_pt = false;
					}
				}

				j++; // Move to the next line
			}

			// Iterate through the polyline points and add to the list
			for (int i = 0; i < (static_cast<int>(polyline_points.size()) - 1); i++)
			{
				// Get i th point and i+1 th point

				dxf_points ln_start_pt = polyline_points[i];
				dxf_points ln_end_pt = polyline_points[i + 1];

				// Add to point list
				// Start Point
				int start_ptid = dxf_points::is_exist(all_dxf_points, ln_start_pt);
				if (start_ptid != -1)
				{
					// Point already exists
					// No need to add

				}
				else
				{
					// Point does not exists (Add start point)
					start_ptid = static_cast<unsigned int>(all_dxf_points.size());
					ln_start_pt.pt_id = start_ptid;
					all_dxf_points.push_back(ln_start_pt);
				}

				// End Point
				int end_ptid = dxf_points::is_exist(all_dxf_points, ln_end_pt);
				if (end_ptid != -1)
				{
					// Point already exists
					// No need to add

				}
				else
				{
					// Point does not exists (Add End point)
					end_ptid = static_cast<unsigned int>(all_dxf_points.size());
					ln_end_pt.pt_id = end_ptid;
					all_dxf_points.push_back(ln_end_pt);
				}

				// Add line
				if (start_ptid != end_ptid)
				{
					dxf_lines line_segment;
					line_segment.startpt_id = start_ptid;
					line_segment.endpt_id = end_ptid;

					// Add line to the list
					if (dxf_lines::is_exist(all_dxf_lines, line_segment) == false)
					{
						all_dxf_lines.push_back(line_segment);
					}
				}
			}
		}
		// iterate line
		j++;
	}

	// Print all the points to ostring stream
	std::ostringstream dxf_input; // Create an ostringstream object

	dxf_input << std::fixed << std::setprecision(9); // Set precision to 6 decimal places

	for (auto& pt : all_dxf_points)
	{
		dxf_input << "node, " << pt.pt_id << ", " << pt.pt_x << ", " << pt.pt_y << "\n";
	}

	int line_id = 0;
	for (auto& ln : all_dxf_lines)
	{
		dxf_input << "line, " << line_id << "," << ln.startpt_id << ", " << ln.endpt_id << ", " << "0" << "\n";
		line_id++;
	}

	// std::string outputStr = output.str(); // Get the string from the ostringstream
	//  std::cout << outputStr; // Print on console to check


	geom.read_dxfdata(dxf_input);

	input_file.close();
}