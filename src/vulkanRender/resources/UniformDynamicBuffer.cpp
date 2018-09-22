#include "UniformDynamicBuffer.h"
#include <iostream>

void* alignedAlloc(size_t size, size_t alignment)
{
	void *data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
	data = _aligned_malloc(size, alignment);
#else 
	int res = posix_memalign(&data, alignment, size);
	if (res != 0)
		data = nullptr;
#endif
	return data;
}

void alignedFree(void* data)
{
#if	defined(_MSC_VER) || defined(__MINGW32__)
	_aligned_free(data);
#else 
	free(data);
#endif
}

void UniformDynamicBuffer::destroy(Device * device)
{
	vkDestroyBuffer(device->handle(), _buffer, nullptr);
	vkFreeMemory(device->handle(), _memory, nullptr);
}

void UniformDynamicBuffer::flushMemory(Device * device, void * alignedAlloc)
{
	void* ptr = nullptr;
	vkMapMemory(device->handle(), _memory, 0, _bufferSize, 0, &ptr);
	memcpy(ptr, alignedAlloc, _bufferSize);
	VkMappedMemoryRange memoryRange = {};
	memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	memoryRange.size = _bufferSize;
	memoryRange.memory = _memory;
	VK_CHECK_RESULT(vkFlushMappedMemoryRanges(device->handle(), 1, &memoryRange));
}

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
	_align = (size / uboAlign) * uboAlign + alignment;
	_bufferSize = num * _align;

	std::pair<VkBuffer, VkDeviceMemory> bufferMemory = device->allocateBuffer(_bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	_buffer = bufferMemory.first;
	_memory = bufferMemory.second;
	/*
	void* ptr = nullptr;
	vkMapMemory(device->handle(), _memory, 0, bufferSize, 0, &ptr);
	for (uint32_t i = 0; i < num; i++) {
		uint64_t* x = (uint64_t*)ptr + (_align * i);
		memcpy(x, data, size);
	}

	VkMappedMemoryRange memoryRange = {};
	memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	memoryRange.size = bufferSize;
	memoryRange.memory = _memory;
	VK_CHECK_RESULT(vkFlushMappedMemoryRanges(device->handle(), 1, &memoryRange));
	*/
	_descriptor.buffer = _buffer;
	_descriptor.offset = 0;
	_descriptor.range = VK_WHOLE_SIZE;
}

UniformDynamicBuffer::UniformDynamicBuffer()
{
}
