#version 450

//in
layout(location = 0) in vec2 uv;
//uniforms
layout(set = 0, binding = 0) uniform sampler2D sprite;
//out
layout(location =0) out vec4 frag_color;
void main()
{
	frag_color = texture(sprite, uv);
}