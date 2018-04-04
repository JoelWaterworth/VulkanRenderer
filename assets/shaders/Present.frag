#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform sampler2D gAlbedoSpec;
layout (binding = 1) uniform sampler2D gPosition;
layout (binding = 2) uniform sampler2D gNormal;

layout (location = 0) in vec3 i_Normal;
layout (location = 1) in vec2 i_uv;

layout (location = 0) out vec4 uFragColor;

struct Light {
	vec4 position;
	vec3 color;
	float radius;
};

layout (binding = 3) uniform UBO {
	Light light[1];
    vec3 viewPos;
} ubo;

void main() {
		// Get G-Buffer values
	vec3 fragPos = texture(gPosition, i_uv).rgb;
	vec3 normal = texture(gNormal, i_uv).rgb;
	vec4 albedo = texture(gAlbedoSpec, i_uv);

	#define lightCount 1
	#define ambient 0.1

	// Ambient part
	vec3 fragcolor  = albedo.rgb * ambient;
	for(int i = 0; i < lightCount; ++i)
	{
		// Vector to light
		vec3 L = ubo.light[i].position.xyz - fragPos;
		// Distance from light to fragment position
		float dist = length(L);

		// Viewer to fragment
		vec3 V = ubo.viewPos.xyz - fragPos;
		V = normalize(V);

		//if(dist < ubo.lights[i].radius)
		{
			// Light to fragment
			L = normalize(L);

			// Attenuation
			float atten = ubo.light[i].radius / (pow(dist, 2.0) + 1.0);

			// Diffuse part
			vec3 N = normalize(normal);
			float NdotL = max(0.0, dot(N, L));
			vec3 diff = ubo.light[i].color * albedo.rgb * NdotL * atten;

			// Specular part
			// Specular map values are stored in alpha of albedo mrt
			vec3 R = reflect(-L, N);
			float NdotR = max(0.0, dot(R, V));
			vec3 spec = ubo.light[i].color * albedo.a * pow(NdotR, 16.0) * atten;

			fragcolor += diff + spec;
		}
	}

  uFragColor = vec4(fragcolor, 1.0);
}