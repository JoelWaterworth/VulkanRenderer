#pragma once
#include <vulkan/vulkan.h>
#include "../shader.h"
#include "../uniformInterface.h"
#include "../device.h"

using namespace std;

struct UniformBinding {
	UniformInterface* uniform;
	uint32_t binding;
	uint32_t set;

	UniformBinding(UniformInterface* Uniform, uint32_t Binding = 0, uint32_t Set = 0) : 
		uniform(Uniform), binding(Binding), set(Set){}
};

class ShaderDescriptor {
public:

	ShaderDescriptor() {};
	ShaderDescriptor(
	  Shader * shader,
	  VkDescriptorPool descriptorPool,
	  vector<VkDescriptorSet> descriptorSets,
	  uint32_t firstSet,
	  uint32_t align
	) :
	  _shader(shader),
	  _descriptorPool(descriptorPool),
	  _descriptorSets(descriptorSets),
	  _firstSet(firstSet),
	  _align(align)
	{};
	
	// create ShaderDescriptor with shader
	ShaderDescriptor(
	  Device * device,
	  Shader* shader,
	  vector<UniformBinding> uniformBuffers,
	  uint32_t setOffset = 0,
	  bool bMakeShaderParent = true,
	  uint32_t align = 0
	);
	
	void destroy(Device* device);
	
	inline vector<VkDescriptorSet>* getDescriptorSets() { return &_descriptorSets; };
	//shader is only supplied if this ShaderDescriptor is used for multiple different shaders
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

