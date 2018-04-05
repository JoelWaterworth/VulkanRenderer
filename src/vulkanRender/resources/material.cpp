#include "material.h"
#include "../util.h"
#include <iostream>

Material::Material(Device * device, Shader * shader, VkDescriptorPool descriptorPool, vector<VkDescriptorSet> descriptorSets, uint32_t firstSet)
{
	_device = device;
	_shader = shader;
	_descriptorPool = descriptorPool;
	_descriptorSets = descriptorSets;
	_firstSet = firstSet;
}

Material::~Material()
{
	vkDestroyDescriptorPool(_device->handle(), _descriptorPool, nullptr);
}

Material * Material::CreateMaterialWithShader(Device * device, Shader * shader, vector<UniformBinding> uniformBuffers, uint32_t setOffset, bool bMakeShaderParent)
{
	uint32_t lastSet = 0;
	std::vector<VkDescriptorPoolSize> pool;
	for (int i = 0; i < uniformBuffers.size(); i++) {
		VkDescriptorPoolSize x = {};
		x.descriptorCount = 1;
		x.type = shader->getShaderLayout()[uniformBuffers[i].set][uniformBuffers[i].binding].type;
		lastSet = (uniformBuffers[i].set > lastSet) ? uniformBuffers[i].set : lastSet;
		pool.push_back(x);
	};
	uint32_t descriptorCount = lastSet - setOffset + 1;

	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.maxSets = 5;
	descriptorPoolInfo.poolSizeCount = pool.size();
	descriptorPoolInfo.pPoolSizes = pool.data();

	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	vkCreateDescriptorPool(device->handle(), &descriptorPoolInfo, nullptr, &descriptorPool);

	vector<VkDescriptorSetLayout> desSetLayouts(descriptorCount);
	for (int i = 0; i < descriptorCount; i++) {
		desSetLayouts[i] = shader->_desSetLayouts[setOffset + i];
	}

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorSetCount = descriptorCount;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.pSetLayouts = desSetLayouts.data();

	vector<VkDescriptorSet> descriptorSet(descriptorCount);

	VK_CHECK_RESULT(vkAllocateDescriptorSets(device->handle(), &allocInfo, descriptorSet.data()));

	vector<VkWriteDescriptorSet> descriptors(uniformBuffers.size());
	for (int i = 0; descriptors.size() > i; i++) {
		descriptors[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptors[i].dstSet = descriptorSet[uniformBuffers[i].set - setOffset];
		descriptors[i].dstBinding = uniformBuffers[i].binding;
		descriptors[i].dstArrayElement = 0;
		descriptors[i].descriptorCount = 1;
		descriptors[i].descriptorType = uniformBuffers[i].uniform->getDescriptorType();
		descriptors[i].pBufferInfo = uniformBuffers[i].uniform->getBufferInfo();
		descriptors[i].pImageInfo = uniformBuffers[i].uniform->getImageInfo();
		descriptors[i].pTexelBufferView = nullptr;
	};

	vkUpdateDescriptorSets(device->handle(), descriptors.size(), descriptors.data(), 0, nullptr);
	return new Material(device, bMakeShaderParent ? shader : nullptr, descriptorPool, descriptorSet, setOffset);
}

void Material::makeCurrent(VkCommandBuffer cmd, Shader* shader) {
	if (_shader == nullptr) {
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->GetPipelineLayout(), _firstSet, _descriptorSets.size(), _descriptorSets.data(), 0, nullptr);
	}
	else {
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _shader->GetPipelineLayout(), _firstSet, _descriptorSets.size(), _descriptorSets.data(), 0, nullptr);
	}
}