#version 450
layout(location = 0) out vec2 uv;
vec4 vertices[6]=
{
	vec4(-0.5, 0.5,		0.0,1.0),
	vec4(-0.5,-0.5,		0.0,0.0),
	vec4( 0.5,-0.5,		1.0,0.0),
	vec4( 0.5,-0.5,		1.0,0.0),
	vec4( 0.5, 0.5,		1.0,1.0),
	vec4(-0.5, 0.5,		0.0,1.0)
};

void main()
{
	gl_Position = vec4(vertices[gl_VertexIndex].xy,1.0,1.0);
	uv = vertices[gl_VertexIndex].zw ;
}