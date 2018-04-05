#include "UniformDynamicBuffer.h"



VkDescriptorType UniformDynamicBuffer::getDescriptorType()
{
	return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
}

VkDescriptorBufferInfo * UniformDynamicBuffer::getBufferInfo()
{
	return &_descriptor;
}

UniformDynamicBuffer::UniformDynamicBuffer(Device * device, size_t size, uint32_t num, const void * data)
{
	VkDeviceSize uboAlign = device->_deviceProperties.limits.minUniformBufferOffsetAlignment;
	VkDeviceSize alignment = (size % uboAlign) > 0 ? uboAlign : 0;
	VkDeviceSize dynamicAlign = (size / uboAlign) * uboAlign + alignment;
	VkDeviceSize bufferSize = num * dynamicAlign;

	std::pair<VkBuffer, VkDeviceMemory> bufferMemory = device->allocateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	_buffer = bufferMemory.first;
	_memory = bufferMemory.second;

	void* ptr = nullptr;
	vkMapMemory(device->handle(), _memory, 0, bufferSize, 0, &ptr);
	for (uint32_t i = 0; i < num; i++) {
		uint64_t* x = (uint64_t*)ptr + (dynamicAlign * i);
		memcpy(x, data, size);
	}
	VkMappedMemoryRange memoryRange = {};
	memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	memoryRange.size = bufferSize;
	memoryRange.memory = _memory;
	vkFlushMappedMemoryRanges(device->handle(), 1, &memoryRange);

	_descriptor.buffer = _buffer;
	_descriptor.offset = 0;
	_descriptor.range = VK_WHOLE_SIZE;
}

UniformDynamicBuffer::UniformDynamicBuffer()
{
}


UniformDynamicBuffer::~UniformDynamicBuffer()
{
}
