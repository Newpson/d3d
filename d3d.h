#pragma once
#include "algebra.h"
#include <stdbool.h>

// SDL SHIT START //
#include <SDL2/SDL.h>
// SDL SHIT END //

typedef struct
{
	vec4 location;
	vec4 rotation;
	vec4 scale;
} d3d_transform;
extern d3d_transform const D3D_TRANSFORM_I;

typedef struct
{
	double fov;
	double ratio;
	double near;
	double far;
} d3d_frustum;

typedef struct
{
	int num_verts;
	int num_faces;
	vec4 *verts;
	int (*faces)[3];
} d3d_mesh;

typedef struct
{
	mat4 frustum;
	mat4 view;
} d3d_camera_cache;

typedef struct
{
	d3d_frustum frustum;
	d3d_transform view;
	d3d_camera_cache cache;
	double sens_loc;
	double sens_rot;
} d3d_camera;
extern d3d_camera const D3D_CAMERA_DEFAULT;

enum d3d_transform_slot
{
	D3D_TRANSFORM_SLOT_A,
	D3D_TRANSFORM_SLOT_B,
	D3D_TRANSFORM_SLOT_COUNT,
};

typedef struct
{
	mat4 transform[D3D_TRANSFORM_SLOT_COUNT];
	vec4 *verts_camera;
	vec4 *verts_frustum;
	size_t num_faces_visible;
	bool *face_visible;
} d3d_object_cache;

// SDL SHIT START //
typedef struct
{
	SDL_Vertex *verts;
	int num_indices;
	int *indices;
} sdl_object_cache;
// SDL SHIT END //

typedef struct
{
	d3d_mesh *mesh;
	d3d_transform view;
	d3d_object_cache cache;

	// SDL SHIT START //
	sdl_object_cache sdl_cache;
	// SDL SHIT END //
} d3d_object;

void d3d_transform_cook(mat4 r, d3d_transform const *T);
void d3d_frustum_cook(mat4 r, d3d_frustum const *P);

d3d_mesh * d3d_mesh_parse(char const *fname);
void d3d_mesh_destroy(d3d_mesh *mesh);

d3d_object * d3d_object_create(char const *fname, d3d_transform transform);
void d3d_object_destroy(d3d_object *object);
