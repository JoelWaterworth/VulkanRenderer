#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

struct Light {
	vec4 position;
	vec3 colour;
	float radius;
};

layout (binding = 0, set = 1) uniform Model {
	mat4 transform;
	vec3 colour;
	float roughness;
	float metallic;
} model;
layout (binding = 0, set = 0) uniform Camera {
	mat4 per;
	mat4 world;
	mat4 model;
	Light lights[4];
	vec3 viewPos;
	int lightCount;
	float gamma;
	float exposure;
} camera;
layout (binding = 1, set = 0) uniform samplerCube samplerIrradiance;
layout (binding = 2, set = 0) uniform sampler2D samplerBRDFLUT;
layout (binding = 3, set = 0) uniform samplerCube prefilteredMap;

layout (location = 0) in vec4 i_pos;
layout (location = 1) in vec3 i_normal;
layout (location = 2) in vec2 i_uv;

layout (location = 0) out vec4 FragColor;

const float PI = 3.14159265359;

vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

// Normal Distribution function --------------------------------------
float D_GGX(float dotNH, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom*denom); 
}

// Geometric Shadowing function --------------------------------------
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// Fresnel function ----------------------------------------------------
vec3 F_Schlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 prefilteredReflection(vec3 R, float roughness)
{
	const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
	float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);
	vec3 a = textureLod(prefilteredMap, R, lodf).rgb;
	vec3 b = textureLod(prefilteredMap, R, lodc).rgb;
	return mix(a, b, lod - lodf);
}

vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness, vec3 albedo)
{
	// Precalculate vectors and dot products	
	vec3 H = normalize (V + L);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);

	// Light color fixed
	vec3 lightColor = vec3(1.0);

	vec3 color = vec3(0.0);

	if (dotNL > 0.0) {
		// D = Normal distribution (Distribution of the microfacets)
		float D = D_GGX(dotNH, roughness); 
		// G = Geometric shadowing term (Microfacets shadowing)
		float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
		vec3 F = F_Schlick(dotNV, F0);		
		vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);		
		vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);			
		color += (kD * albedo / PI + spec) * dotNL;
	}

	return color;
}

void main()
{		  
	vec3 WorldPos = i_pos.xyz;
	vec3 normal = i_normal;
	vec3 albedo = model.colour;
	float metallic = model.metallic;
	float roughness =  model.roughness;


	vec3 N = normalize(normal);
	vec3 V = normalize(camera.viewPos - WorldPos);
	vec3 R = reflect(V, N); 

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0.0);
	for(int i = 0; i < camera.lightCount; i++) {
		vec3 L = normalize(camera.lights[i].position.xyz - WorldPos.xyz);
		Lo += specularContribution(L, V, N, F0, metallic, roughness, albedo);
	}  

	vec2 brdf = texture(samplerBRDFLUT, vec2(max(dot(normal, V), 0.0), roughness)).rg;
	vec3 reflection = prefilteredReflection(R, roughness).rgb;	
	vec3 irradiance = texture(samplerIrradiance, N).rgb;

	// Diffuse based on irradiance
	vec3 diffuse = irradiance * albedo;	

	vec3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);

	// Specular reflectance
	vec3 specular = reflection * (F * brdf.x + brdf.y);

	// Ambient part
	vec3 kD = 1.0 - F;
	kD *= 1.0 - metallic;	  
	vec3 ambient = (kD * diffuse + specular);
	
	vec3 color = ambient + Lo;

	// Tone mapping
	color = Uncharted2Tonemap(color * camera.exposure);
	color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	// Gamma correction
	color = pow(color, vec3(1.0f / camera.gamma));

    FragColor = vec4(color, 1.0);
}  