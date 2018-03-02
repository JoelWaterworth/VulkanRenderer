#include "UniformBuffer.h"



UniformBuffer::UniformBuffer(EnDevice * device, vk::DescriptorBufferInfo descriptor, vk::DeviceMemory memory, vk::Buffer buffer)
{
	_device = device;
	_descriptor = descriptor;
	_memory = memory;
	_buffer = buffer;
}

UniformBuffer::~UniformBuffer()
{
	_device->destroyBuffer(_buffer);
	_device->freeMemory(_memory);
}

vk::DescriptorType UniformBuffer::getDescriptorType()
{
	return vk::DescriptorType::eUniformBuffer;
}

vk::DescriptorBufferInfo* UniformBuffer::getBufferInfo()
{
	return &_descriptor;
}

UniformBuffer * UniformBuffer::CreateUniformBufferBody(EnDevice * device, size_t size, const void* data)
{
	std::pair<vk::Buffer, vk::DeviceMemory> bufferMemory = device->allocateBuffer(size, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible);
	vk::Buffer buffer = bufferMemory.first;
	vk::DeviceMemory memory = bufferMemory.second;

	void* ptr = device->mapMemory(memory, 0, size);
	memcpy(ptr, data, size);
	device->unmapMemory(memory);
	return new UniformBuffer(device, vk::DescriptorBufferInfo().setBuffer(buffer).setOffset(0).setRange(VK_WHOLE_SIZE), memory, buffer);
}
