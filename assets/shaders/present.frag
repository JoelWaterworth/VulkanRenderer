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
	vec3 albedo;
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

layout (binding = 4, set = 0) uniform sampler2D TColor;
layout (binding = 5, set = 0) uniform sampler2D TNormal;
layout (binding = 6, set = 0) uniform sampler2D TRoughness;
layout (binding = 7, set = 0) uniform sampler2D TMetallic;
layout (binding = 8, set = 0) uniform sampler2D TAO;

layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec4 outColor;

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
void main()
{
	vec3 albedo = pow(texture(TColor, inUV).rgb, vec3(2.2));

	float metallic = texture(TMetallic, inUV).r;
	float roughness = texture(TRoughness, inUV).r;
	float ao = texture(TAO, inUV).r;

	vec3 N = normalize(inNormal);
    vec3 V = normalize(camera.viewPos - inWorldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	           
    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < camera.lightCount; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(camera.lights[i].position.xyz - inWorldPos);
        vec3 H = normalize(V + L);
        float distance    = length(camera.lights[i].position.xyz - inWorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = camera.lights[i].colour * attenuation;        
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H,  roughness);        
        float G   = GeometrySmith(N, V, L,  roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular     = numerator / max(denominator, 0.001);  
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    }   
  
    vec3 ambient = vec3(0.001) * albedo * ao;
    vec3 color = ambient + Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    outColor = vec4(color, 1.0);
}