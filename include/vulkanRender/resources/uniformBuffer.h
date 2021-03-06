#pragma once
#include "../uniformInterface.h"
#include "resource.h"

class UniformBuffer : public UniformInterface, public Resource
{
public:
	UniformBuffer(Device* device, VkDescriptorBufferInfo descriptor, VkDeviceMemory memory, VkBuffer buffer, VkDeviceSize size);
	UniformBuffer() {};
	void destroy(Device* device);
	template <class T>
	static UniformBuffer CreateUniformBuffer(Device* device, T data);

	void update(Device* device, const void* data);

	virtual VkDescriptorType getDescriptorType();
	virtual VkDescriptorBufferInfo* getBufferInfo();

private:
	static UniformBuffer CreateUniformBufferBody(Device* device, size_t size, const void* data);
	VkDescriptorBufferInfo _descriptor;
	VkDeviceMemory _memory = VK_NULL_HANDLE;
	VkBuffer _buffer = VK_NULL_HANDLE;
	VkDeviceSize _size = 0;
};

template<class T>
inline UniformBuffer UniformBuffer::CreateUniformBuffer(Device * device, T data)
{
	size_t size = sizeof(data);
	return UniformBuffer::CreateUniformBufferBody(device, size, &data);
}
