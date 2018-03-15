#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform sampler2D dTexture;
//layout (binding = 0) uniform UBO {
//	vec3 colour;
//} ubo;

layout (location = 0) in vec3 o_Normal;
layout (location = 1) in vec2 o_uv;

layout (location = 0) out vec4 uFragColor;

void main() {
	uFragColor = texture(dTexture, o_uv);
//    uFragColor = vec4(ubo.colour, 1.0);
}