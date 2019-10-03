# Mini Renderer

A mini rasterization-based renderer built entirely in C++ from scratch without any graphics libraries. Done as a semester long project for Csci580 at the University of Southern California. 

A PDF file has been included with some outputs that demonstrate the various facets of the renderer, as will be outlined in the project description section.

## Notes

It is important to note that for potential copyright infringment purposes, the base framework (which includes creating a window and reading in .asc input files) has not been included in this repository, and thus the .cpp file is intended to be only a demonstration of C++ programming abilities. Futhermore, the entire core functionality is all in a single file for logistical purposes as per requirements of the class (making it easier for the instructors to look at and grade all code), and thus this file is not intended to demonstrate any skills related to class design or software architecture. To reiterate, the code in this repository is meant purely to demonstrate understanding of 3D graphics and math concepts, and the ability to code them to produce a meaningful result.

## Project description

The goal of this project was to create a 3D renderer from scratch that implements the basic functionality of any renderer as follows:

* Rasterization: Implements the scan-line method of rasterization, which uses the slope of a triangle edge to compute endpoints of each line and draw pixels in between.
* Transformations: Implements rotation, scaling and translation of vertices and normals, while at the same time handling conversion all the way from model space to screen space.
* Shading: Implements the Gouraud shader, which interpolates the per-vertex color, as well as the Phong shader, which interpolates based on normals. Note that these settings are made in the main function of the base framework which has been omitted for potential copyright purposes.
* Texturing
* Anti aliasing: Using jittered supersampling.

## Built With

* C++
