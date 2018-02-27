# Surface Registration Tool

This software register two surface meshes by iterative closest point method.

GUI Screenshot: 
![GUI Screenshot](./GUI.jpg "GUI Screenshot")

## Usage
The program includes both GUI and commandline interface. IO supports STL,VTP and VTK files.

CLI is under development.

## Initial Transform
Registration initialization is specified by user matrix displayed by the 4x4 matrix table.
User may alter the transformation directly on the matrix table or by changing the sliders.

## Registration methods
The surface first passthrought the initial transform matrix, then registered by following algorithms:

1. Iterative Closest Point (ICP)

	Users may refer to the documentation [here](https://www.vtk.org/doc/nightly/html/classvtkIterativeClosestPointTransform.html)

2. Principal Component Analysis / Iterative Closest Point (PCA-ICP)
	PCA is adopted for initialization then implement ICP registration. Suitable for asymmetric surfaces. Much computation expensive than method 1. Recommended for registering surfaces with small number of points.

## Test Files
Test surfaces could be found in data folder