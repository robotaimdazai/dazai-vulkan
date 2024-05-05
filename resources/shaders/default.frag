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
    if (color.a == 0.0)
        discard;

    // Set the output color to the specified color (76, 156, 184)
    vec3 specifiedColor = vec3(76.0 / 255.0, 156.0 / 255.0, 184.0 / 255.0);
    frag_color = vec4(specifiedColor, 1.0);
}
