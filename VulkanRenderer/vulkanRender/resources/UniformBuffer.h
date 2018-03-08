#pragma once
#include "../UniformInterface.h"
#include "Resource.h"

class UniformBuffer : public UniformInterface, public Resource
{
public:
	UniformBuffer(EnDevice* device, vk::DescriptorBufferInfo descriptor, vk::DeviceMemory memory, vk::Buffer buffer);
	virtual ~UniformBuffer();
	template <class T>
	static UniformBuffer* CreateUniformBuffer(EnDevice* device, T data);

	virtual vk::DescriptorType getDescriptorType();
	virtual vk::DescriptorBufferInfo* getBufferInfo();

private:
	static UniformBuffer* CreateUniformBufferBody(EnDevice* device, size_t size, const void* data);
	vk::DescriptorBufferInfo _descriptor;
	vk::DeviceMemory _memory;
	vk::Buffer _buffer;
};

template<class T>
inline UniformBuffer * UniformBuffer::CreateUniformBuffer(EnDevice * device, T data)
{
	size_t size = sizeof(data);
	return UniformBuffer::CreateUniformBufferBody(device, size, &data);
}
