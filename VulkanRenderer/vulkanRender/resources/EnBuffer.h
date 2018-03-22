#pragma once
#include "Resource.h"
#include <vulkan/vulkan.hpp>
#include "../EnDevice.h"

class EnBuffer :
	public Resource
{
public:
	EnBuffer();
	static EnBuffer* Create(EnDevice* device, vk::BufferUsageFlags usage, vk::DeviceSize size, vk::MemoryPropertyFlags flags);
	vk::Buffer buffer;
	virtual void bindMemory(EnDevice* device, vk::DeviceMemory memory, uint64_t localOffset);
	virtual void destroy(EnDevice* device);
	void* mapMemory(EnDevice* device);
	void unMapMemory(EnDevice* device);
	void setObjectName(EnDevice* device, const char* name);
private:
};

