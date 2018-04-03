#include "uniformBuffer.h"



UniformBuffer::UniformBuffer(Device * device, VkDescriptorBufferInfo descriptor, VkDeviceMemory memory, VkBuffer buffer)
{
	_device = device;
	_descriptor = descriptor;
	_memory = memory;
	_buffer = buffer;
}

UniformBuffer::~UniformBuffer()
{
	vkDestroyBuffer(_device->handle(), _buffer, nullptr);
	vkFreeMemory(_device->handle(), _memory, nullptr);
}

VkDescriptorType UniformBuffer::getDescriptorType()
{
	return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}

VkDescriptorBufferInfo* UniformBuffer::getBufferInfo()
{
	return &_descriptor;
}
/*
UniformBuffer * UniformBuffer::CreateUniformBufferBody(Device * device, size_t size, const void* data)
{
	std::pair<VkBuffer, VkDeviceMemory> bufferMemory = device->allocateBuffer(size, VkBufferUsageFlagBits::eUniformBuffer, VkMemoryPropertyFlagBits::eHostVisible);
	VkBuffer buffer = bufferMemory.first;
	VkDeviceMemory memory = bufferMemory.second;

	void* ptr = nullptr;
	vkMapMemory(device->handle(), memory, 0, size, 0, &ptr);
	memcpy(ptr, data, size);
	vkUnmapMemory(device->handle(), memory);
	VkDescriptorBufferInfo descriptorInfo = {};
	descriptorInfo.buffer = buffer;
	descriptorInfo.offset = 0;
	descriptorInfo.range = VK_WHOLE_SIZE;
	return new UniformBuffer(device, descriptorInfo, memory, buffer);
}*/
