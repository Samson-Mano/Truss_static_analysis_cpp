# Plane truss finite element solver

Description:
This repository contains a C++ project that implements a Finite Element Method (FEM) solver for plane truss structures. The FEM is a widely used numerical method for analyzing and solving structural engineering problems, such as trusses, beams, and frames. The solver in this project can compute the displacements, reactions, and internal forces of a given plane truss structure subjected to external loads.

## Features:

- Implementation of the FEM for solving plane truss problems
- Support for various boundary conditions, including inclined pinned, roller, and free supports
- Input file format for defining truss geometry, material properties, and loads
- Visualization of truss geometry, displacements, and internal forces using OpenGL
- Post-processing capabilities.
- Zoom in/out (Ctrl + scroll), Pan operation (Ctrl + Right click drag), Zoom to fit (Ctrl + F)

## Usage:
- Download Truss_static_cpp_portable_version.7z to your PC. Extract and run the exe file to open the application.
- Read How to use Plane Truss Finite Element Solver.pdf.
- Import the truss geometry, define the material properties, constraints, and loads.
- Run the FE solver, which computes the truss displacements, reactions, and internal forces
- Visualize the results using the included OpenGL-based GUI, which provides options for displaying truss geometry, displacements, and internal forces
- Post-process the results as needed for further analysis

## Dependencies:

- C++ compiler that supports C++17 or higher
- GLFW and GLEW libraries for OpenGL windowing and rendering
- ImGui library for creating GUI
- Eigen library for linear algebra operations
- Other necessary libraries for file I/O, data visualization, and post-processing as needed

## License:
This project is open-source and released under the MIT license. Feel free to use, modify, and distribute the code as per the terms of the license.

## Example 1
![steel tower](/Images/steel_tower_ex1.PNG)
![steel_tower_model](/Images/steel_tower_ex1_model_view.PNG)
![steel tower deflection](/Images/steel_tower_ex1_defl_view.PNG)
![steel tower member force](/Images/steel_tower_ex1_mforce_view.PNG)

## Example 2
![overhang](/Images/overhang_ex2.PNG)
![overhang model view](/Images/overhang_ex2_model_view.PNG)
![overhang deflection](/Images/overhang_ex2_defl_view.PNG)
![overhang member force](/Images/overhang_ex2_mforce_view.PNG)

## Example 3
![railroad bridge](/Images/railroad_ex3.PNG)
![railroad model view](/Images/railroad_ex3_model_view.PNG)
![railroad deflection](/Images/railroad_ex3_defl_view.PNG)
![railroad member force](/Images/railroad_ex3_mforce_view.PNG)

## Status: Complete
## To DO List
- ~~Create the ImGUI menu bar events~~
- ~~Create the mouse events~~
- ~~Import the model data~~
- ~~Create class to handle the model data~~
- ~~Set up the vertex buffer, index buffer, buffer layout etc~~
- ~~Create a basic vertex & fragment shader~~
- ~~Define the model matrix as well as translate, zoom and rotate matrix for vertex shader~~
- ~~Paint the truss model with basic elements (as points and lines)~~
- ~~Fit the imported geometry with proper geometry scale~~
- ~~Pan, zoom and zoom to fit operation~~
- ~~Implement circle for nodes (instead of points) texture for nodes~~
- ~~UI for adding constraint~~
- ~~UI for adding load~~
- ~~Add load and constraint to the model by clicking on the node~~
- ~~Text in GUI to show load value, length, node number, member number etc.~~
- ~~import/export model in native format~~
- ~~UI for updating material~~
- ~~Build solver~~
- ~~Color profile for the model environment~~
- ~~Map the results to nodes and elements~~
- ~~Postprocessing using Dynamic draw~~
- ~~Animate results~~
- ~~Finalize the resource imports~~
