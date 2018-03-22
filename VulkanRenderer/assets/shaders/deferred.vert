#version 450 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec3 i_normal;
layout (location = 2) in vec2 i_uv;

layout (location = 0) out vec3 o_pos;
layout (location = 1) out vec3 o_nor;
layout (location = 2) out vec2 o_uv;

void main()
{
	o_pos = i_position;
	o_nor = i_normal;
	o_uv = i_uv;
    gl_Position = vec4(o_pos, 1.0f);
}