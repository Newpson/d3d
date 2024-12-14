#include "algebra.h"
#include <math.h>

vec4 vec4_sum(vec4 *u, vec4 *v)
{
	return (vec4) {u->x+v->x, u->y+v->y, u->z+v->z, u->w+v->w};
}

vec4 vec4_negate(vec4 *u)
{
	return (vec4) {-u->x, -u->y, -u->z, -u->w};
}

vec4 vec4_diff(vec4 *u, vec4 *v)
{
	return (vec4) {u->x-v->x, u->y-v->y, u->z-v->z, u->w-v->w};
}

void mat4_multiply(mat4 r, mat4 a, mat4 b)
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
		{
			r[i].at[j] = 0.0;
			for (int k = 0; k < 4; ++k)
				r[i].at[j] += a[k].at[j] * b[i].at[k];
		}
}

void mat4_transpose(mat4 r, mat4 m)
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			r[i].at[j] = m[j].at[i];
}

void mat4_apply(vec4 *r, mat4 m, vec4 *v)
{
	r->x = m[0].at[0]*v->x + m[1].at[0]*v->y + m[2].at[0]*v->z + m[3].at[0]*v->w;
	r->y = m[0].at[1]*v->x + m[1].at[1]*v->y + m[2].at[1]*v->z + m[3].at[1]*v->w;
	r->z = m[0].at[2]*v->x + m[1].at[2]*v->y + m[2].at[2]*v->z + m[3].at[2]*v->w;
	r->w = m[0].at[3]*v->x + m[1].at[3]*v->y + m[2].at[3]*v->z + m[3].at[3]*v->w;
}
