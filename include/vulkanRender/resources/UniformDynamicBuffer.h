#pragma once

#include "../uniformInterface.h"
#include <vulkan/vulkan.h>
#include "resource.h"

class UniformDynamicBuffer : public UniformInterface, public Resource
{
public:
	UniformDynamicBuffer(Device * device, size_t size, uint32_t num, const void* data);
	UniformDynamicBuffer();
	virtual ~UniformDynamicBuffer();

	template <class T>
	static UniformDynamicBuffer Create(Device* device, vector<T> data);

	virtual VkDescriptorType getDescriptorType();
	virtual VkDescriptorBufferInfo* getBufferInfo();
	inline  VkDeviceSize getAlign() { return _align; };

private:
	VkDescriptorBufferInfo _descriptor = {};
	VkDeviceMemory _memory = nullptr;
	VkBuffer _buffer = nullptr;
	VkDeviceSize _align = 0;
	Device* _device = nullptr;
};

template<class T>
inline UniformDynamicBuffer UniformDynamicBuffer::Create(Device * device, vector<T> data)
{
	size_t size = sizeof(T);
	return UniformDynamicBuffer(device, size, data.size(), data.data());
}
