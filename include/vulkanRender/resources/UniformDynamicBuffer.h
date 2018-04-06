#pragma once

#include "../uniformInterface.h"
#include <vulkan/vulkan.h>
#include "util.h"
#include "resource.h"

void* alignedAlloc(size_t size, size_t alignment);
void alignedFree(void* data);

class UniformDynamicBuffer : public UniformInterface, public Resource
{
public:
	UniformDynamicBuffer(Device * device, size_t size, uint32_t num, const void* data);
	UniformDynamicBuffer();
	virtual ~UniformDynamicBuffer();

	template <class T>
	static UniformDynamicBuffer Create(Device* device, vector<T> data);

	void flushMemory(Device * device, void* alignedAlloc);

	virtual VkDescriptorType getDescriptorType();
	virtual VkDescriptorBufferInfo* getBufferInfo();
	inline  VkDeviceSize getAlign() { return _align; };

private:
	VkDescriptorBufferInfo _descriptor = {};
	VkDeviceMemory _memory = nullptr;
	VkBuffer _buffer = nullptr;
	VkDeviceSize _align = 0;
	VkDeviceSize _bufferSize = 0;
	Device* _device = nullptr;
};

template<class T>
UniformDynamicBuffer UniformDynamicBuffer::Create(Device * device, vector<T> data)
{
	size_t size = sizeof(T);
	auto u = UniformDynamicBuffer(device, size, data.size(), data.data());
	auto allo = alignedAlloc(u._bufferSize, u._align);
	for (int i = 0; i < data.size(); i++) {
		T* dst = (T*)(((uint64_t)allo + (i * u._align)));
		*dst = data[i];
	}
	u.flushMemory(device, allo);
	alignedFree(allo);
	return u;
}
