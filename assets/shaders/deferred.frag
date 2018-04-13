#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0, set = 2) uniform sampler2D dColour;
layout (binding = 1, set = 2) uniform sampler2D dNormal;
layout (binding = 2, set = 2) uniform sampler2D dRoughness;
layout (binding = 3, set = 2) uniform sampler2D dMetallic;
layout (binding = 4, set = 2) uniform sampler2D dAO;

layout (location = 0) in vec4 i_pos;
layout (location = 1) in vec3 i_nor;
layout (location = 2) in vec2 i_uv;

layout (location = 0) out vec4 uFragColor;
layout (location = 1) out vec4 uFragPosition;
layout (location = 2) out vec3 uFragNormal;
layout (location = 3) out float uFragMat;
layout (location = 4) out float uFragRough;
layout (location = 5) out float uFragAO;


void main() {
	uFragColor = vec4(texture(dColour, i_uv).rgb, 1.0);
	uFragPosition = i_pos;
	uFragNormal = i_nor;
	uFragMat = 0.1f;
	uFragRough = texture(dMetallic, i_uv).r;
	uFragAO = texture(dAO, i_uv).r;
}