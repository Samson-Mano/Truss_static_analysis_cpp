# Status: In progress
## To DO List
- ~~Create the ImGUI menu bar events~~
- ~~Create the mouse events~~
- ~~Import the model data~~
- ~~Create class to handle the model data~~
- Set up the vertex buffer, index buffer, buffer layout etc
- Create a basic vertex & fragment shader
- Define the model matrix as well as translate, zoom and rotate matrix for vertex shader
- Paint the truss model with basic elements (as points and lines)
- Implement circle for nodes (instead of points)
- Implement rectangle for lines (instead of primitive line)



# Plane truss finite element solver

Description:
This repository contains a C++ project that implements a Finite Element Method (FEM) solver for plane truss structures. The FEM is a widely used numerical method for analyzing and solving structural engineering problems, such as trusses, beams, and frames. The solver in this project can compute the displacements, reactions, and internal forces of a given plane truss structure subjected to external loads. This project is part of my effort in learning C++.

## Features:

- Implementation of the FEM for solving plane truss problems
- Support for various boundary conditions, including pinned, roller, and free supports
- Input file format for defining truss geometry, material properties, and loads
- Visualization of truss geometry, displacements, and internal forces using OpenGL
- Post-processing capabilities, such as plotting of results and export of data

## Usage:

- Define the truss geometry, material properties, and loads in the input file using the specified format
- Run the solver executable, which reads the input file and computes the truss displacements, reactions, and internal forces
- Visualize the results using the included OpenGL-based GUI, which provides options for displaying truss geometry, displacements, and internal forces
- Post-process the results as needed, such as plotting them or exporting them for further analysis
- Read How_to_use.Pdf for more information.

## Dependencies:

- C++ compiler that supports C++11 or higher
- GLFW and GLEW libraries for OpenGL windowing and rendering
- ImGui library for creating GUI
- Eigen library for linear algebra operations
- Other necessary libraries for file I/O, data visualization, and post-processing as needed

## License:
This project is open-source and released under the MIT license. Feel free to use, modify, and distribute the code as per the terms of the license.
