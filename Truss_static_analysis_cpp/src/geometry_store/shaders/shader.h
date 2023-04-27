#pragma once

#include<iostream>
#include<fstream>
#include<string>

#include<GL\glew.h>
#include<GLFW\glfw3.h>

#include<glm\glm.hpp>
#include<glm\vec2.hpp>
#include<glm\vec3.hpp>
#include<glm\vec4.hpp>
#include<glm\mat4x4.hpp>
#include<glm\gtc\type_ptr.hpp>

// https://github.com/Headturna/OpenGL-C---Tutorials/blob/master/Shader.h
// https://github.com/MichaelMoroz/ShaderToy2CPP/blob/master/src/Shaders.cpp

class shader
{
private:
	// Member variables
	GLuint id; // OpenGL program ID
	std::string loadShaderSource(char* fileName); // Function to load shader source from file
	GLuint loadShader(GLenum type, char* fileName); // Function to load a shader of given type
	void linkProgram(GLuint vertexShader, GLuint fragmentShader); // Function to link the shader program

public:
	// Constructors/Destructors
	shader(char* vertexFile, char* fragmentFile); // Constructor that takes vertex and fragment shader file names
	~shader(); // Destructor to clean up OpenGL resources

	// Shader usage functions
	void Use(); // Function to use the shader program
	void unUse(); // Function to unuse the shader program

	// Uniform setting functions
	void setUniform(std::string name, float X, float Y); // Function to set a 2D float uniform
	void setUniform(std::string name, float X, float Y, float Z); // Function to set a 3D float uniform
	void setUniform(std::string name, float X, float Y, float Z, float W); // Function to set a 4D float uniform
	void setUniform(std::string name, float X); // Function to set a float uniform
	void setUniform(std::string name, int X); // Function to set an integer uniform
	void setUniform(std::string name, glm::mat3 X, bool transpose); // Function to set a mat3 uniform
	void setUniform(std::string name, glm::mat4 X, bool transpose);  // Function to set a mat4 uniform
	void setUniform(std::string name, glm::vec4 X); // Function to set a vec4 uniform
	void setUniform(std::string name, glm::vec3 X); // Function to set a vec3 uniform
	void setUniform(std::string name, glm::vec2 X); // Function to set a vec2 uniform
	void setUniform(int i, GLuint tid); // Function to set a texture uniform
};
