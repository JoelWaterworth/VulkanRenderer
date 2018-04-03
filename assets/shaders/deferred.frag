#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform sampler2D dTexture;

layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec3 i_nor;
layout (location = 2) in vec2 i_uv;

layout (location = 0) out vec4 uFragColor;
layout (location = 1) out vec3 uFragPosition;
layout (location = 2) out vec3 uFragNormal;

void main() {
	uFragColor = texture(dTexture, i_uv);
	uFragPosition = i_pos;
	uFragNormal = i_nor;
}