#include "algebra.h"
#include "d3d.h"
#include "error.h"
#include "memory.h"

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// SDL SHIT START //
#include <SDL2/SDL.h>
// SDL SHIT END //

d3d_transform const D3D_TRANSFORM_I =
{
	.location = {0.0, 0.0, 0.0},
	.rotation = {0.0, 0.0, 0.0},
	.scale    = {1.0, 1.0, 1.0},
};

d3d_frustum const D3D_FRUSTUM_DEFAULT =
{
	.fov = RAD(85.0),
	.ratio = 4.0 / 3.0,
	.near = 0.001,
	.far = 35.0,
};

d3d_camera const D3D_CAMERA_DEFAULT =
{
	.view = D3D_TRANSFORM_I,
	.frustum = D3D_FRUSTUM_DEFAULT,
	.sens_loc = 0.1,
	.sens_rot = 0.003,
};

static
void parse_nop(d3d_mesh *, char *)
{
	return;
}

static
void parse_first_v(d3d_mesh *mesh, char *)
{
	++mesh->num_verts;
}

static
void parse_first_f(d3d_mesh *mesh, char *)
{
	++mesh->num_faces;
}

static
void parse_second_v(d3d_mesh *mesh, char *line)
{
	static int i = 0;
	if (mesh == NULL)
	{
		i = 0;
		return;
	}

	for (int j = 0; j < 3; ++j)
		mesh->verts[i].at[j] = strtof(line, &line);
	mesh->verts[i].w = 1.0;
	++i;
}

static
void parse_second_f(d3d_mesh *mesh, char *line)
{
	static int i = 0;
	if (mesh == NULL)
	{
		i = 0;
		return;
	}

	for (int j = 0; j < 3; ++j)
	{
		mesh->faces[i][j] = strtol(line, &line, 10) - 1;
		if (*line == '/')
			while (isgraph(*line))
				++line;
	}

	++i;
}

static
void parse_start(d3d_mesh *mesh, char *)
{
	mesh->verts = memory_take(mesh->num_verts * sizeof(vec4));
	mesh->faces = memory_take(mesh->num_faces * sizeof(int [3]));
	DEBUGF("Ready to read: %d vertices, %d faces",
			mesh->num_verts, mesh->num_faces);
}

d3d_mesh * d3d_mesh_parse(const char *fname)
{
	d3d_mesh *mesh = memory_take(sizeof(d3d_mesh));
	FAILURE_ALLOC(mesh);

	FILE *file = fopen(fname, "r");
	FAILURE_IF(file == NULL, NULL, "Failed to open file \"%s\"", fname);

	/* kind of state machine */
	enum d3d_parse_state
	{
		D3D_GEOMETRY_V,
		D3D_GEOMETRY_F,
		D3D_HOOK,
	};
	void (*parse[2][3])(d3d_mesh *, char *) =
	{
		{
			[D3D_GEOMETRY_V] = parse_first_v,
			[D3D_GEOMETRY_F] = parse_first_f,
			[D3D_HOOK] = parse_start,
		},
		{
			[D3D_GEOMETRY_V] = parse_second_v,
			[D3D_GEOMETRY_F] = parse_second_f,
			[D3D_HOOK] = parse_nop,
		},
	};

	/* reset indices */
	parse_second_v(NULL, NULL);
	parse_second_f(NULL, NULL);

	char buffer[256], *i;
	for (int pass = 0; pass < 2; ++pass)
	{
		while (fgets(buffer, 256, file))
		{
			i = buffer;
			while (isblank(*i)) { ++i; }
			i = strtok(i, "\x09\x20"); // read until space or tab
			if (strncmp(i, "v", 2) == 0) // vertex
				parse[pass][D3D_GEOMETRY_V](mesh, i + 2);
			else if (strncmp(i, "f", 2) == 0) // face
				parse[pass][D3D_GEOMETRY_F](mesh, i + 2);
		}
		parse[pass][D3D_HOOK](mesh, NULL);
		rewind(file);
	}

	fclose(file);

	return mesh;
}

void d3d_mesh_destroy(d3d_mesh *mesh)
{
	// Destroy operation is not supported because memory pool is used
	return;
}

d3d_object * d3d_object_create(char const *fname, d3d_transform transform)
{
	d3d_object *object = memory_take(sizeof(d3d_object));
	FAILURE_ALLOC(object);
	d3d_mesh *mesh = d3d_mesh_parse(fname);
	FAILURE_IF(mesh == NULL, NULL);
	object->mesh = mesh;
	object->view = transform;

	object->cache.verts_camera = memory_take(mesh->num_verts * sizeof(vec4));
	FAILURE_ALLOC(object->cache.verts_camera);
	object->cache.verts_frustum = memory_take(mesh->num_verts * sizeof(vec4));
	FAILURE_ALLOC(object->cache.verts_camera);
	object->cache.face_visible = memory_take(mesh->num_faces * sizeof(bool));
	FAILURE_ALLOC(object->cache.face_visible);

	// SDL SHIT START //
	object->sdl_cache.verts = memory_take(mesh->num_verts * sizeof(SDL_Vertex));
	FAILURE_ALLOC(object->sdl_cache.verts);
	// max usage of indidices if all the polygons are visible
	object->sdl_cache.indices = memory_take(mesh->num_faces * 3 * sizeof(int));
	FAILURE_ALLOC(object->sdl_cache.indices);
	// SDL SHIT END //
	return object;
}

void d3d_object_destroy(d3d_object *object)
{
	// Destroy operation is not supported because memory pool is used
	return;
}

void d3d_frustum_cook(mat4 r, d3d_frustum const *P)
{
	r[0] = (vec4) {1.0/P->ratio/tan(P->fov/2.0), 0.0, 0.0, 0.0};
	r[1] = (vec4) {0.0, 1.0/tan(P->fov/2.0), 0.0, 0.0};
	r[2] = (vec4) {0.0, 0.0, -(P->far+P->near)/(P->far-P->near), -1.0};
	r[3] = (vec4) {0.0, 0.0, -2*P->far*P->near/(P->far-P->near), 0.0};
}

void d3d_rotation_cook(mat4 r, vec4 const *R)
{
	static double cosy, siny, cosp, sinp, cosr, sinr;
	
	cosy = cos(R->yaw);
	siny = sin(R->yaw);
	cosp = cos(R->pitch);
	sinp = sin(R->pitch);
	cosr = cos(R->roll);
	sinr = sin(R->roll);

	r[0] = (vec4) {
		.x = cosr * cosy - sinr * sinp * siny,
		.y = sinr * cosy + cosr * sinp * siny,
		.z = -cosp * siny,
		.w = 0.0,
	};

	r[1] = (vec4) {
		.x = -sinr * cosp,
		.y = cosr * cosp,
		.z = sinp,
		.w = 0.0,
	};

	r[2] = (vec4) {
		.x = cosr * siny + sinr * sinp * cosy,
		.y = sinr * siny - cosr * sinp * cosy,
		.z = cosp * cosy,
		.w = 0.0,
	};

	r[3] = (vec4) {
		.x = 0.0,
		.y = 0.0,
		.z = 0.0,
		.w = 1.0,
	};
}
void d3d_scale_cook(mat4 r, vec4 const *S)
{
	r[0] = (vec4) { .x = S->x, .y = 0.0, .z = 0.0, .w = 0.0 };
	r[1] = (vec4) { .x = 0.0, .y = S->y, .z = 0.0, .w = 0.0 };
	r[2] = (vec4) { .x = 0.0, .y = 0.0, .z = S->z, .w = 0.0 };
	r[3] = (vec4) { .x = 0.0, .y = 0.0, .z = 0.0, .w = 1.0 };
}

void d3d_location_cook(mat4 r, vec4 const *L)
{
	r[0] = (vec4) { .x = 1.0, .y = 0.0, .z = 0.0, .w = 0.0 };
	r[1] = (vec4) { .x = 0.0, .y = 1.0, .z = 0.0, .w = 0.0 };
	r[2] = (vec4) { .x = 0.0, .y = 0.0, .z = 1.0, .w = 0.0 };
	r[3] = (vec4) { .x = L->x, .y = L->y, .z = L->z, .w = 1.0 };
}

void d3d_transform_cook(mat4 r, d3d_transform const *T)
{
	static mat4 slota, slotb, slotc;

	d3d_scale_cook(slota, &T->scale);
	d3d_rotation_cook(slotb, &T->rotation);
	mat4_multiply(slotc, slotb, slota); // R * S
	d3d_location_cook(slota, &T->location);
	mat4_multiply(r, slotc, slota); // (R * S) * L
}
