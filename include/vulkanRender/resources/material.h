#pragma once
#include "resource.h"
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include "../shader.h"
#include "../uniformInterface.h"

using namespace std;

struct UniformBinding {
	UniformInterface* uniform;
	uint32_t binding;
	uint32_t set;
};

class Material : public Resource
{
public:
	Material(Device * device, Shader * shader, VkDescriptorPool descriptorPool, vector<VkDescriptorSet> descriptorSets);
	~Material();
	static Material* CreateMaterialWithShader(Device * device, Shader* shader, vector<UniformBinding> uniformBuffers);
	inline vector<VkDescriptorSet>* getDescriptorSets() { return &_descriptorSets; };
private:
	Shader* _shader;
	vector<VkDescriptorSetLayout> _descriptorSetLayout;
	VkDescriptorPool _descriptorPool;
	vector<VkDescriptorSet> _descriptorSets;

	Device* _device;
};

