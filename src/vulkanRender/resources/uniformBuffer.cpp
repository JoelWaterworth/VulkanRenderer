#include "uniformBuffer.h"
#include <cstring>


UniformBuffer::UniformBuffer(Device * device, VkDescriptorBufferInfo descriptor, VkDeviceMemory memory, VkBuffer buffer, VkDeviceSize size)
{
	_descriptor = descriptor;
	_memory = memory;
	_buffer = buffer;
	_size = size;
}


void UniformBuffer::destroy(Device * device)
{
	vkDestroyBuffer(device->handle(), _buffer, nullptr);
	vkFreeMemory(device->handle(), _memory, nullptr);
}

void UniformBuffer::update(Device * device, const void * data)
{
	void* ptr = VK_NULL_HANDLE;
	vkMapMemory(device->handle(), _memory, 0, _size, 0, &ptr);
	std::memcpy(ptr, data, _size);
	vkUnmapMemory(device->handle(), _memory);
}

VkDescriptorType UniformBuffer::getDescriptorType()
{
	return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}

VkDescriptorBufferInfo* UniformBuffer::getBufferInfo()
{
	return &_descriptor;
}

UniformBuffer UniformBuffer::CreateUniformBufferBody(Device * device, size_t size, const void* data)
{
	std::pair<VkBuffer, VkDeviceMemory> bufferMemory = device->allocateBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VkBuffer buffer = bufferMemory.first;
	VkDeviceMemory memory = bufferMemory.second;

	void* ptr = nullptr;
	vkMapMemory(device->handle(), memory, 0, size, 0, &ptr);
	std::memcpy(ptr, data, size);
	vkUnmapMemory(device->handle(), memory);
	VkDescriptorBufferInfo descriptorInfo = {};
	descriptorInfo.buffer = buffer;
	descriptorInfo.offset = 0;
	descriptorInfo.range = size;
	return UniformBuffer(device, descriptorInfo, memory, buffer, size);
}
