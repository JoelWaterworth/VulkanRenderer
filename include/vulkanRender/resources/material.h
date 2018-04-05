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
	Material() {};
	Material(Shader * shader, VkDescriptorPool descriptorPool, vector<VkDescriptorSet> descriptorSets, uint32_t firstSet, uint32_t align);
	~Material();
	virtual void destroy(Device* device);
	static Material CreateMaterialWithShader(Device * device, Shader* shader, vector<UniformBinding> uniformBuffers, uint32_t setOffset = 0, bool bMakeShaderParent = true, uint32_t align = 0);
	inline vector<VkDescriptorSet>* getDescriptorSets() { return &_descriptorSets; };
	//shader is only supplied if this material is used for multiple different shaders
	void makeCurrent(VkCommandBuffer cmd, Shader* shader = nullptr);
	void makeCurrentAlign(VkCommandBuffer cmd, uint32_t index, Shader* shader = nullptr);
private:
	Shader* _shader = nullptr;
	vector<VkDescriptorSetLayout> _descriptorSetLayout;
	VkDescriptorPool _descriptorPool;
	vector<VkDescriptorSet> _descriptorSets;
	uint32_t _firstSet = 0;
	uint32_t _align = 0;
};

