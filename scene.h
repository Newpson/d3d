#pragma once
#include "d3d.h"

typedef struct
{
	size_t num_objects_max;
	size_t num_objects;
	d3d_object **objects;
	d3d_camera camera;
} d3d_scene;

d3d_scene * d3d_scene_create(size_t num_objects);
void d3d_scene_render(d3d_scene *);
size_t d3d_scene_object_add(d3d_scene *, d3d_object *);
