#version 450 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 i_position;
layout (location = 0) in vec3 i_normal;
layout (location = 0) in vec2 i_uv;

layout (location = 0) out vec3 o_Normal;
layout (location = 1) out vec2 o_uv;

void main()
{
	o_Normal = i_normal;
	o_uv = (i_uv * 0.5) + vec2(0.5, 0.5);
    gl_Position = vec4(i_position, 1.0f);
}