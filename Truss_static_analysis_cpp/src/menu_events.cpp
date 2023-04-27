#include "menu_events.h"


menu_events::menu_events()
{
}

menu_events::~menu_events()
{
}

void menu_events::update_event(menu_item m_ck, geom_store* geom)
{
	// Menu item m is clicked
	switch (m_ck)
	{
	case import_varai2d:
		import_varai2d_geometry(geom);
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


void menu_events::import_varai2d_geometry(geom_store* geom)
{
	std::string file_path = ShowFileDialog();
	std::cout << "Selected File: " << file_path << std::endl;

	// Create an unordered_map to store nodes with ID as key
	std::unordered_map<int, nodes_store> nodeMap;
	// Create an unordered_map to store lines with ID as key
	std::unordered_map<int, lines_store> lineMap;

	// Open the input file
	std::ifstream input_file(file_path, std::ifstream::in);
	if (!input_file.is_open()) {
		std::cerr << "Failed to open input file: " << file_path << std::endl;
		return;
	}
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

	int j = 0, i = 0;

	// Process the lines
	while (j < lines.size())
	{
		std::cout << "Line: " << lines[j] << std::endl;
		// Check for the start of nodes input
		if (lines[j].find("[+] End Points") != std::string::npos)
		{
			int num_nodes;
			// Read the number of nodes
			std::stringstream ss(lines[j]);
			std::string token;
			std::getline(ss, token, ','); // Get the string "[+] End Points"
			std::getline(ss, token, ','); // Get the number of nodes as a string
			num_nodes = std::stoi(token) + j; // Convert the string to an integer

			// Read and store the nodes
			for (i = j; i < num_nodes; i++)
			{
				int node_id;
				float x, y;

				std::stringstream ss(lines[i + 1]);
				std::string token;

				std::getline(ss, token, ','); // read the node ID
				node_id = std::stoi(token);

				std::getline(ss, token, ','); // read the x-coordinate
				x = std::stof(token);

				std::getline(ss, token, ','); // read the y-coordinate
				y = std::stof(token);

				// Create nodes_store object and store in nodeMap
				nodes_store node(node_id, glm::vec3(x, y, 0.0f));
				nodeMap[node_id] = node;
				j++;
			}
		}
		// Check for the start of lines input
		else if (lines[j].find("[+] Lines") != std::string::npos) {
			int num_lines;
			// Read the number of nodes
			std::stringstream ss(lines[j]);
			std::string token;
			std::getline(ss, token, ','); // Get the string "[+] Lines"
			std::getline(ss, token, ','); // Get the number of nodes as a string
			num_lines = std::stoi(token) + j; // Convert the string to an integer

			// Read and store the lines
			for (i = j; i < num_lines; i++)
			{
				int line_id, start_node_id, end_node_id;
				std::stringstream ss(lines[i + 1]);
				std::string token;

				std::getline(ss, token, ','); // read the line ID
				line_id = std::stoi(token);

				std::getline(ss, token, ','); // read the start node ID
				start_node_id = std::stoi(token);

				std::getline(ss, token, ','); // read the end node ID
				end_node_id = std::stoi(token);

				// Create lines_store object using references to startNode and endNode
				lines_store line(line_id, nodeMap[start_node_id], nodeMap[end_node_id]);
				lineMap[line_id] = line;
				j++;
			}
		}

		// iterate line
		j++;
	}

	// Re-instantitize geom_store object using the nodeMap and lineMap
	geom->deleteResources();
	*geom=geom_store(nodeMap, lineMap);
}