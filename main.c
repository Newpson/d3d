#define SDL_MAIN_HANDLED
#include "d3d.h"
#include "error.h"
#include "memory.h"
#include "scene.h"

#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <SDL2/SDL.h>

d3d_scene * scene_setup(void)
{
	d3d_scene *scene = d3d_scene_create(5);
	FAILURE_IF(scene == NULL, NULL, "Failed to create scene");

	// ~100K polygons
	// https://free3d.com/3d-model/cockroach-v1--788882.html
	// d3d_object *cockroach = d3d_object_create("assets/cockroach.obj", D3D_TRANSFORM_I);
	// FAILURE_IF(cockroach == NULL, NULL);
	// d3d_transform rot = {
	// 	.location = {0.0, 0.0, -1.5},
	// 	.rotation = {RAD(30), RAD(180), 0.0},
	// 	.scale    = {1.0, 1.0, 1.0},
	// };
	// mat4 c_rot; d3d_transform_cook(c_rot, &rot);
	// vec4 res;
	// for (int i = 0; i < model->mesh->num_verts; ++i)
	// {
	// 	mat4_apply(&res, c_rot, &model->mesh->verts[i]);
	// 	model->mesh->verts[i] = res;
	// }
	// d3d_scene_object_add(scene, cockroach);

	// ~1.5KK polygons
	// https://free3d.com/3d-model/bugatti-chiron-2017-model-31847.html
	// d3d_object *bugatti = d3d_object_create("assets/bugatti.obj", D3D_TRANSFORM_I);
	// FAILURE_IF(bugatti == NULL, NULL);
	// d3d_scene_object_add(scene, bugatti);
	
	d3d_object *suzanne = d3d_object_create("assets/suzanne.obj", D3D_TRANSFORM_I);
	FAILURE_IF(suzanne == NULL, NULL);
	d3d_scene_object_add(scene, suzanne);

	return scene;
}

static
bool should_redraw(Uint32 frametime)
{
	static Uint32 last = 0, now;
	now = SDL_GetTicks();
	if (now - last > frametime)
	{
		last = now;
		return true;
	}
	return false;
}

int main(int argc, char *argv[])
{

	SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	SDL_SetRelativeMouseMode(true);
	omp_set_num_threads(4);

	SDL_Window *window = SDL_CreateWindow("d3d",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			800, 600, SDL_WINDOW_OPENGL);
	FAILURE_GOTO(window == NULL, error_window, "Failed to create window");
		
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
			SDL_RENDERER_ACCELERATED);
	FAILURE_GOTO(renderer == NULL, error_renderer, "Failed to create renderer");
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	FAILURE_GOTO((memory_allocate(128 * 1024 * 1024)) == 0, error_memory);

	d3d_scene *scene = scene_setup();
	FAILURE_GOTO(scene == NULL, error_scene);
	DEBUGF("Memory usage: %u bytes", memory_usage());

	bool running = true;
	SDL_Event event;
	const Uint8 *pressed;
	double time_begin, time_end;
	double cosy, siny;
	while (running)
	{
		while (SDL_PollEvent(&event))
			if (event.type == SDL_QUIT)
				running = false;
			else if (event.type == SDL_MOUSEMOTION)
			{
				scene->camera.view.rotation.yaw += scene->camera.sens_rot * event.motion.xrel;
				scene->camera.view.rotation.pitch += scene->camera.sens_rot * event.motion.yrel;
				if (scene->camera.view.rotation.pitch >= RAD(90))
					scene->camera.view.rotation.pitch = RAD(90);
				else if (scene->camera.view.rotation.pitch <= -RAD(90))
					scene->camera.view.rotation.pitch = -RAD(90);
			}
			
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		time_begin = omp_get_wtime();
		d3d_scene_render(scene);
		time_end = omp_get_wtime();

		// cockroach dance
		// scene->objects[0]->view.location.y = 3 * fabs(sin(8 * omp_get_wtime()));
		// scene->objects[0]->view.rotation.yaw += 0.12;

		for (int i = 0; i < scene->num_objects; ++i)
		{
			SDL_RenderGeometry(renderer, NULL,
				scene->objects[i]->sdl_cache.verts,
				scene->objects[i]->mesh->num_verts,
				scene->objects[i]->sdl_cache.indices,
				scene->objects[i]->sdl_cache.num_indices
			);
		}

		pressed = SDL_GetKeyboardState(NULL);
		
		cosy = cos(scene->camera.view.rotation.yaw);
		siny = sin(scene->camera.view.rotation.yaw);
		if (pressed[SDL_SCANCODE_W])
		{
			scene->camera.view.location.z += scene->camera.sens_loc * cosy;
			scene->camera.view.location.x -= scene->camera.sens_loc * siny;
		}
		if (pressed[SDL_SCANCODE_S])
		{
			scene->camera.view.location.z -= scene->camera.sens_loc * cosy;
			scene->camera.view.location.x += scene->camera.sens_loc * siny;
		}
		if (pressed[SDL_SCANCODE_A])
		{
			scene->camera.view.location.z += scene->camera.sens_loc * siny;
			scene->camera.view.location.x += scene->camera.sens_loc * cosy;
		}
		if (pressed[SDL_SCANCODE_D])
		{
			scene->camera.view.location.z -= scene->camera.sens_loc * siny;
			scene->camera.view.location.x -= scene->camera.sens_loc * cosy;
		}
		if (pressed[SDL_SCANCODE_SPACE])
			scene->camera.view.location.y -= scene->camera.sens_loc;
		if (pressed[SDL_SCANCODE_LSHIFT])
			scene->camera.view.location.y += scene->camera.sens_loc;

		SDL_RenderPresent(renderer);

		while (!should_redraw(16)); // 13ms ~= 75 fps; 16ms ~= 60 fps; 33ms ~= 30 fps
	}

error_scene:
	memory_free();
error_memory:
	SDL_DestroyRenderer(renderer);
error_renderer:
	SDL_DestroyWindow(window);
error_window:
	SDL_Quit();

	return EXIT_SUCCESS;
}

