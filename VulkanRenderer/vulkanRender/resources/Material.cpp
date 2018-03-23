#include "Material.h"
#include "../util.h"
#include <iostream>

Material::Material(Device * device, Shader * shader, VkDescriptorPool descriptorPool, VkDescriptorSet descriptorSet)
{
	_device = device;
	_shader = shader;
	_descriptorPool = descriptorPool;
	_descriptorSet = descriptorSet;
}

Material::~Material()
{
	vkDestroyDescriptorPool(_device->handle(), _descriptorPool, nullptr);
}

Material * Material::CreateMaterialWithShader(Device * device, Shader * shader, vector<UniformBinding> uniformBuffers)
{
	std::vector<VkDescriptorPoolSize> pool(shader->getTypes().size());
	for (int i = 0; i < pool.size(); i++) {
		pool[i].descriptorCount = 1;
		pool[i].type = shader->getTypes()[i];
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.maxSets = 1;
	descriptorPoolInfo.poolSizeCount = pool.size();
	descriptorPoolInfo.pPoolSizes = pool.data();

	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	vkCreateDescriptorPool(device->handle(), &descriptorPoolInfo, nullptr, &descriptorPool);

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorSetCount = 1;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.pSetLayouts = shader->getDesSetLayout().data();

	VkDescriptorSet descriptorSet;

	vkAllocateDescriptorSets(device->handle(), &allocInfo, &descriptorSet);

	vector<VkWriteDescriptorSet> descriptors(uniformBuffers.size());
	for (int i = 0; descriptors.size() > i; i++) {
		descriptors[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptors[i].dstSet = descriptorSet;
		descriptors[i].dstBinding = uniformBuffers[i].binding;
		descriptors[i].dstArrayElement = 0;
		descriptors[i].descriptorCount = 1;
		descriptors[i].descriptorType = uniformBuffers[i].uniform->getDescriptorType();
		descriptors[i].pBufferInfo = uniformBuffers[i].uniform->getBufferInfo();
		descriptors[i].pImageInfo = uniformBuffers[i].uniform->getImageInfo();
	};

	vkUpdateDescriptorSets(device->handle(), descriptors.size(), descriptors.data(), 0, nullptr);
	return new Material(device, shader, descriptorPool, descriptorSet);
}
