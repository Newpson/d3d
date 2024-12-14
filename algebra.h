#pragma once
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795028L
#endif
#include <math.h>

#define RAD(deg) ((deg)/180.0*M_PI)

typedef union
{
	struct
	{
		double x;
		double y;
		double z;
		double w;
	};
	struct
	{
		double pitch;
		double yaw;
		double roll;
		double _;
	};
	struct
	{
		double r;
		double g;
		double b;
		double a;
	};
	double at[4];
} vec4;

typedef vec4 mat4[4];

vec4 vec4_negate(vec4 *u);
vec4 vec4_sum(vec4 *u, vec4 *v);
vec4 vec4_diff(vec4 *u, vec4 *v);

// r : result
// a, b : matrices
// (order : a * b)
void mat4_multiply(mat4 r, mat4 a, mat4 b);

// r : result
// m : matrix
// v : vector 
// (order : m * v)
void mat4_apply(vec4 *r, mat4 m, vec4 *v);

void mat4_transpose(mat4 r, mat4 m);
