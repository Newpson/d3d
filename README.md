![cockroach](https://github.com/user-attachments/assets/8195ec4a-fcf2-4be0-9792-9ce23aeb8600)

demo3d
======
- **What is it?**
This is a portable program that allows you to load Wavefront .obj files into a scene and apply any transformations to them.

- **What's the purpose? There are many programs that do this.**
First of all, it was done to practice 3D skills (but I'm not really interested in CGI, just for fun!). By coincidence, it later became one of the assignments in the unsiversity. And this project was used twice to complete them.

- **You said it uses software rendering. What about performance?**
Hard to say. The thickest model I have ever loaded was about 1.5 million polygons. And it ran pretty smooth on Ryzen 3 2200g (4 threads). By the way, rendering is not entirely software as it uses SDL2 to render computed 2D geometry.

- **How does it work?**
Here is the simplified pipeline:
	1. Handle input;
	1. Compute new camera matrix;
	1. Recompute all the objects (do not call any SDL-specific functions here as this section is supposed to be parallel):
		1. Cook local transformation matrix (if needed);
		1. Apply local transoformation and then camera transformation to all the vertices;
		1. Compute normals (z component) for all the polygons;
		1. Mark the polygons that will be rendered later (if normal directed to the camera);
		1. Apply projection transformation to all the vertices;
		1. Generate SDL geometry from projected data (color of vertex = distance between vertex and camera (z component));
	1. Render SDL geometry.

- **Will you continue to develop this project?**
Probably not due to the lack of time. Here are some unfinished things:
	- texturing support;
	- better lighting;
	- wiser cache (internal cache, not CPU cache) usage;
	- common geometric pool (more convenient for parallel calculation, better performance);
	- pseudo z-buffer for polygons (polygon sorting in the pool);
	- code refactoring;
	- and more...

