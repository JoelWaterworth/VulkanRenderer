#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (binding = 0) uniform UBO {
	vec3 colour;
} ubo;

layout (location = 0) out vec4 uFragColor;

void main() {
    uFragColor = vec4(ubo.colour, 1.0);
}