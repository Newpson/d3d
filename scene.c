#include "d3d.h"
#include "error.h"
#include "memory.h"
#include "scene.h"

#include <omp.h>
#include <stddef.h>

d3d_scene * d3d_scene_create(size_t num_objects)
{
	d3d_scene *scene = memory_take(sizeof(d3d_scene));
	FAILURE_ALLOC(scene);
	scene->objects = memory_take(num_objects * sizeof(d3d_object));
	FAILURE_ALLOC(scene->objects);
	scene->camera = D3D_CAMERA_DEFAULT;

	// immutable during the rendering pipeline
	d3d_frustum_cook(
		scene->camera.cache.frustum,
		&scene->camera.frustum
	);


	scene->num_objects_max = num_objects;
	scene->num_objects = 0;
	return scene;
}

static
double d3d_normal_z(vec4 *v1, vec4 *v2, vec4 *v3)
{
	return
		(v2->x - v1->x) * (v3->y - v2->y) -
		(v2->y - v1->y) * (v3->x - v2->x);
}


static
bool d3d_in_frustum(vec4 *v1, vec4 *v2, vec4 *v3, double near, double far)
{
	return
		v1->w >= near && v1->w <= far &&
		v2->w >= near && v2->w <= far &&
		v3->w >= near && v3->w <= far;
}

static
void d3d_scene_compute(d3d_scene *scene)
{
	static d3d_object *obj;

	d3d_transform_cook(
		scene->camera.cache.view,
		&scene->camera.view
	);

	for (size_t i = 0; i < scene->num_objects; ++i)
	{
		obj = scene->objects[i];

		// local transform
		d3d_transform_cook(
			obj->cache.transform[D3D_TRANSFORM_SLOT_A],
			&obj->view
		);

		// (camera x local) transform
		mat4_multiply(
			obj->cache.transform[D3D_TRANSFORM_SLOT_B],
			scene->camera.cache.view,
			obj->cache.transform[D3D_TRANSFORM_SLOT_A]
		);

		// apply (camera x local) and frustum to all vertices
		// (as separate transforms)
#pragma omp parallel for
		for (int j = 0; j < obj->mesh->num_verts; ++j)
		{
			mat4_apply(
				&obj->cache.verts_camera[j],
				obj->cache.transform[D3D_TRANSFORM_SLOT_B],
				&obj->mesh->verts[j]
			);
			mat4_apply(
				&obj->cache.verts_frustum[j],
				scene->camera.cache.frustum,
				&obj->cache.verts_camera[j]
			);
		}

		// mark the polygons that will be rendered
		int num_faces_visible = 0;
#pragma omp parallel for reduction(+ : num_faces_visible)
		for (int j = 0; j < obj->mesh->num_faces; ++j)
		{
			bool visible = ((d3d_normal_z(
				&obj->cache.verts_camera[obj->mesh->faces[j][0]],
				&obj->cache.verts_camera[obj->mesh->faces[j][1]],
				&obj->cache.verts_camera[obj->mesh->faces[j][2]]
			) > 0) && d3d_in_frustum(
				&obj->cache.verts_frustum[obj->mesh->faces[j][0]],
				&obj->cache.verts_frustum[obj->mesh->faces[j][1]],
				&obj->cache.verts_frustum[obj->mesh->faces[j][2]],
				scene->camera.frustum.near,
				scene->camera.frustum.far
			));

			obj->cache.face_visible[j] = visible;

			if (visible)
				++num_faces_visible;
		}
		obj->cache.num_faces_visible = num_faces_visible;

// TODO make parallel too
		// SDL SHIT START //
		obj->sdl_cache.num_indices = 3 * obj->cache.num_faces_visible;
		for (int j = 0, k = 0; j < obj->mesh->num_faces; ++j)
		{
			if (obj->cache.face_visible[j])
			{
				obj->sdl_cache.indices[k + 0] = obj->mesh->faces[j][0];
				obj->sdl_cache.indices[k + 1] = obj->mesh->faces[j][1];
				obj->sdl_cache.indices[k + 2] = obj->mesh->faces[j][2];
				k += 3;
			}
		}

#pragma omp parallel for
		for (int j = 0; j < obj->mesh->num_verts; ++j)
		{
			double threshold = -scene->camera.frustum.far;
			double color = 1.0 - (obj->cache.verts_camera[j].z < threshold ? 1.0 :
				obj->cache.verts_camera[j].z / threshold);
			obj->sdl_cache.verts[j] = (SDL_Vertex)
			{
				.position = {
					800.0 * (obj->cache.verts_frustum[j].x /
						obj->cache.verts_frustum[j].w + 0.5),
					600.0 * (-obj->cache.verts_frustum[j].y /
						obj->cache.verts_frustum[j].w + 0.5),
				},
				.color = {
					255 * color,
					255 * color,
					255 * color,
					255
				}
			};
		}
		// SDL SHIT END //
	}
// #pragma omp barrier
}

void d3d_scene_render(d3d_scene *scene)
{
	d3d_scene_compute(scene);
}

size_t d3d_scene_object_add(d3d_scene *scene, d3d_object *object)
{
	FAILURE_IF(scene->num_objects >= scene->num_objects_max, 0,
		"Senpai~~ I can't fit em all! U.~");
	scene->objects[scene->num_objects] = object;
	DEBUGF("Added successfully");
	return ++scene->num_objects;
}
