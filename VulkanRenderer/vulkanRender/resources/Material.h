#pragma once
#include "Resource.h"
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include "../Shader.h"
#include "../UniformInterface.h"

using namespace std;

struct UniformBinding {
	UniformInterface* uniform;
	uint32_t binding;
};

class Material : public Resource
{
public:
	Material(Device * device, Shader * shader, VkDescriptorPool descriptorPool, VkDescriptorSet descriptorSet);
	~Material();
	static Material* CreateMaterialWithShader(Device * device, Shader* shader, vector<UniformBinding> uniformBuffers);
	inline VkDescriptorSet* getDescriptorSet() { return &_descriptorSet; };
private:
	Shader* _shader;
	vector<VkDescriptorSetLayout> _descriptorSetLayout;
	VkDescriptorPool _descriptorPool;
	VkDescriptorSet _descriptorSet;

	Device* _device;
};

