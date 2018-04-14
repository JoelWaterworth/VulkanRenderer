#version 450

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec3 i_normal;
layout (location = 2) in vec2 i_uv;

layout (location = 0) out vec4 o_pos;
layout (location = 1) out vec3 o_nor;
layout (location = 2) out vec2 o_uv;

void main()
{
	o_pos = vec4(i_position, 1.0);
	o_nor = i_normal;
	o_uv = i_uv;
    gl_Position = vec4(i_position, 1.0f);
}