#pragma once
#include "resource.h"
#include <vulkan/vulkan.h>
#include "../shader.h"
#include "../uniformInterface.h"

using namespace std;

struct UniformBinding {
	UniformInterface* uniform;
	uint32_t binding;
	uint32_t set;

	UniformBinding(UniformInterface* Uniform, uint32_t Binding = 0, uint32_t Set = 0) : 
		uniform(Uniform), binding(Binding), set(Set){}
};

class Material : public Resource
{
public:
	Material(Device * device, Shader * shader, VkDescriptorPool descriptorPool, vector<VkDescriptorSet> descriptorSets, uint32_t firstSet);
	~Material();
	static Material* CreateMaterialWithShader(Device * device, Shader* shader, vector<UniformBinding> uniformBuffers, uint32_t setOffset = 0, bool bMakeShaderParent = true);
	inline vector<VkDescriptorSet>* getDescriptorSets() { return &_descriptorSets; };
	//shader is only supplied if this material is used for multiple different shaders
	void makeCurrent(VkCommandBuffer cmd, Shader* shader = nullptr);
private:
	Shader* _shader;
	vector<VkDescriptorSetLayout> _descriptorSetLayout;
	VkDescriptorPool _descriptorPool;
	vector<VkDescriptorSet> _descriptorSets;
	Device* _device;
	uint32_t _firstSet;
};

