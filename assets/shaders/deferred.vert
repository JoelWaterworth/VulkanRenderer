#version 450 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec3 i_normal;
layout (location = 2) in vec2 i_uv;

layout (location = 0) out vec4 o_pos;
layout (location = 1) out vec3 o_nor;
layout (location = 2) out vec2 o_uv;

layout (binding = 0, set = 0) uniform Camera {
	mat4 m;
} camera;

layout (binding = 0, set = 1) uniform Model {
	mat4 m;
} model;

void main()
{
	o_pos = camera.m * model.m * vec4(i_position, 1);
	o_nor = i_normal;
	o_uv = i_uv;
    gl_Position = o_pos;
}