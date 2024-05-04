#version 450
#include "../../src/engine/shared_structs.h"

layout(set =0, binding = 0) uniform global_ubo
{
	global_data g_data;
};

layout(set =0, binding = 1) readonly buffer transforms
{
	transform g_transforms[];
};

transform t = g_transforms[gl_InstanceIndex];

layout(location = 0) out vec2 uv;
vec4 vertices[4]=
{
	vec4(t.x, t.y,								0.0,0.0),
	vec4(t.x, t.y + t.size_y,					0.0,1.0),
	vec4(t.x + t.size_x, t.y + t.size_y,		1.0,1.0),
	vec4(t.x + t.size_x, t.y,					1.0,0.0),
};

void main()
{
	vec2 pos = 2.0 * vec2(vertices[gl_VertexIndex].x/g_data.width,vertices[gl_VertexIndex].y/g_data.height) -1.0;
	gl_Position = vec4(pos,1.0,1.0);
	uv = vertices[gl_VertexIndex].zw ;
}