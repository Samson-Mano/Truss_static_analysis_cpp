#include "geom_store.h"

geom_store::geom_store()
	: is_geometry_set(false),
	is_geometry_loaded(false),
	min_b(glm::vec3(0)),
	max_b(glm::vec3(0)),
	geom_bound(glm::vec3(0)),
	center(glm::vec3(0)),
	shaderProgram(0),
	vao(),vbo(),ibo()
{
	// Empty constructor
}

void geom_store::create_geometry(const std::unordered_map<int, nodes_store>& nodeMap, 
	std::unordered_map<int, lines_store>& lineMap)
{
	// Constructor
	this->nodeMap = nodeMap;
	this->lineMap = lineMap;

	// Set the boundary of the geometry
	std::pair<glm::vec3, glm::vec3> result = findMinMaxXY(nodeMap);
	min_b = result.first;
	max_b = result.second;
	geom_bound = max_b - min_b;

	// Set the center of the geometry
	center = findGeometricCenter(nodeMap);

	// Geometry is loaded
	is_geometry_loaded = true;

	set_geometry();
}

geom_store::~geom_store()
{
	// Destructor
	deleteResources();
}

void geom_store::deleteResources()
{
	is_geometry_loaded = false;
	is_geometry_set = false;
	// Call the destructor for each nodes_store object in the nodeMap
	for (auto& node : nodeMap) {
		node.second.~nodes_store();
	}
	// Call the destructor for each lines_store object in the lineMap
	for (auto& line : lineMap) {
		line.second.~lines_store();
	}
	// Clear the nodeMap and lineMap
	nodeMap.clear();
	lineMap.clear();
}

std::pair<glm::vec3, glm::vec3> geom_store::findMinMaxXY(const std::unordered_map<int, nodes_store>& nodeMap)
{
	// Initialize min and max values to first node in map
	auto firstNode = nodeMap.begin()->second.node_pt;
	glm::vec3 minXY = glm::vec3(firstNode.x, firstNode.y, firstNode.z);
	glm::vec3 maxXY = minXY;

	// Loop through all nodes in map and update min and max values
	for (auto it = nodeMap.begin(); it != nodeMap.end(); ++it)
	{
		const auto& node = it->second.node_pt;
		if (node.x < minXY.x)
		{
			minXY.x = node.x;
		}
		if (node.y < minXY.y)
		{
			minXY.y = node.y;
		}
		if (node.x > maxXY.x)
		{
			maxXY.x = node.x;
		}
		if (node.y > maxXY.y)
		{
			maxXY.y = node.y;
		}
	}

	// Return pair of min and max values
	return { minXY, maxXY };
}


glm::vec3 geom_store::findGeometricCenter(const std::unordered_map<int, nodes_store>& nodeMap)
{
	// Function returns the geometric center of the nodes
	// Initialize the sum with zero
	glm::vec3 sum(0);

	// Sum the points
	for (auto it = nodeMap.begin(); it != nodeMap.end(); ++it)
	{
		sum += it->second.node_pt;
	}
	return sum / static_cast<float>(nodeMap.size());
}

void geom_store::set_geometry()
{
	if (is_geometry_loaded == false)
		return;



	/*shader main_shader("/shaders/geom_vertex_shader.vert", "/shaders/geom_frag_shader.frag");
	main_shader.Bind();*/
	// Set the geometry
// Define the vertices of the triangle
	float vertices[] = {
		// Position          // Color
		-0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
		 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
	};

	// Define the indices of the triangle
	unsigned int indices[] = {
		0, 1, 2
	};

	vao.createVertexArray();
	vbo.createVertexBuffer(vertices, 3 * 6 * sizeof(float));

	// Create an index buffer object and bind it to the vertex array object
	ibo.createIndexBuffer(indices, 3);

	VertexBufferLayout layout;
	layout.AddFloat(3);  // Position
	layout.AddFloat(3);  // Color

	vao.AddBuffer(vbo, layout);

	// Geometry is set
	is_geometry_set = true;
}

void geom_store::paint_geometry()
{
	if (is_geometry_set == false)
		return;

	//// Create a shader program
	//const char* vertexShaderSource =
	//	"#version 330 core\n"
	//	"layout (location = 0) in vec3 aPos;\n"
	//	"layout (location = 1) in vec3 aColor;\n"
	//	"out vec3 fColor;\n"
	//	"void main()\n"
	//	"{\n"
	//	"    gl_Position = vec4(aPos, 1.0);\n"
	//	"    fColor = aColor;\n"
	//	"}\n";
	//const char* fragmentShaderSource =
	//	"#version 330 core\n"
	//	"in vec3 fColor;\n"
	//	"out vec4 FragColor;\n"
	//	"void main()\n"
	//	"{\n"
	//	"    FragColor = fColor;\n"
	//	"}\n";

	//// Create Vertex Shader Object and get its reference
	//GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//// Attach Vertex Shader source to the Vertex Shader Object
	//glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	//// Compile the Vertex Shader into machine code
	//glCompileShader(vertexShader);

	//// Create Fragment Shader Object and get its reference
	//GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//// Attach Fragment Shader source to the Fragment Shader Object
	//glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	//// Compile the Vertex Shader into machine code
	//glCompileShader(fragmentShader);

	//// Create Shader Program Object and get its reference
	//GLuint shaderProgram = glCreateProgram();
	//// Attach the Vertex and Fragment Shaders to the Shader Program
	//glAttachShader(shaderProgram, vertexShader);
	//glAttachShader(shaderProgram, fragmentShader);
	//// Wrap-up/Link all the shaders together into the Shader Program
	//glLinkProgram(shaderProgram);

	//// Delete the now useless Vertex and Fragment Shader objects
	//glDeleteShader(vertexShader);
	//glDeleteShader(fragmentShader);


	// Clean the back buffer and assign the new color to it
	// glClear(GL_COLOR_BUFFER_BIT);
	// glUseProgram(shaderProgram);

	vao.Bind();
	ibo.Bind();

	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

	vao.UnBind();
	ibo.UnBind();
}
