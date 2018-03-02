#include "Material.h"
#include "../util.h"
#include <iostream>

Material::Material(EnDevice * device, Shader * shader, vk::DescriptorPool descriptorPool, vk::DescriptorSet descriptorSet)
{
	_device = device;
	_shader = shader;
	_descriptorPool = descriptorPool;
	_descriptorSet = descriptorSet;
}

Material::~Material()
{
	_device->destroyDescriptorPool(_descriptorPool);
}

Material * Material::CreateMaterialWithShader(EnDevice * device, Shader * shader, vector<UniformBinding> uniformBuffers)
{
	std::vector<vk::DescriptorPoolSize> pool(shader->getTypes().size());
	for (int i = 0; i < pool.size(); i++) {
		pool[i] = vk::DescriptorPoolSize().setDescriptorCount(1).setType(shader->getTypes()[i]);
	};

	auto const descriptorPoolInfo = vk::DescriptorPoolCreateInfo()
		.setMaxSets(1)
		.setPoolSizeCount(pool.size())
		.setPPoolSizes(pool.data());

	vk::DescriptorPool descriptorPool = device->createDescriptorPool(descriptorPoolInfo);

	auto const allocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(descriptorPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(shader->getDesSetLayout().data());

	vk::DescriptorSet descriptorSet;

	device->allocateDescriptorSets(&allocInfo, &descriptorSet);

	vector<vk::WriteDescriptorSet> descriptors(uniformBuffers.size());
	for (int i = 0; descriptors.size() > i; i++) {
		descriptors[i] = vk::WriteDescriptorSet()
			.setDstSet(descriptorSet)
			.setDstBinding(uniformBuffers[i].binding)
			.setDstArrayElement(0)
			.setDescriptorCount(1)
			.setDescriptorType(uniformBuffers[i].uniform->getDescriptorType())
			.setPBufferInfo(uniformBuffers[i].uniform->getBufferInfo());
	};

	device->updateDescriptorSets(descriptors.size(), descriptors.data(), 0, nullptr);
	return new Material(device, shader, descriptorPool, descriptorSet);
}
