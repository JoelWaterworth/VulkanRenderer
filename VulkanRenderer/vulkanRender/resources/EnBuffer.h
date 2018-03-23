#pragma once
#include "Resource.h"
#include <vulkan/vulkan.hpp>
#include "../Device.h"

class EnBuffer :
	public Resource
{
public:
	EnBuffer();
	static EnBuffer* Create(Device* device, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags flags);
	VkBuffer buffer;
	virtual void bindMemory(Device* device, VkDeviceMemory memory, uint64_t localOffset);
	virtual void destroy(Device* device);
	void* mapMemory(Device* device);
	void unMapMemory(Device* device);
	void setObjectName(Device* device, const char* name);
private:
};

