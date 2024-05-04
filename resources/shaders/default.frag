#version 450

//in
layout(location = 0) in vec2 uv;
//uniforms
layout(set = 0, binding = 2) uniform sampler2D sprite;
//out
layout(location =0) out vec4 frag_color;
void main()
{
	vec4 color = texture(sprite, uv);
	if(color.a == 0)
		discard;
	frag_color = color;
}