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
	Material(EnDevice * device, Shader * shader, vk::DescriptorPool descriptorPool, vk::DescriptorSet descriptorSet);
	~Material();
	static Material* CreateMaterialWithShader(EnDevice * device, Shader* shader, vector<UniformBinding> uniformBuffers);
	inline vk::DescriptorSet getDescriptorSet() const { return _descriptorSet; };
private:
	Shader* _shader;
	vector<vk::DescriptorSetLayout> _descriptorSetLayout;
	vk::DescriptorPool _descriptorPool;
	vk::DescriptorSet _descriptorSet;
};

