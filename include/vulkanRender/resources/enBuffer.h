#pragma once
#include "resource.h"
#include <vulkan/vulkan.h>

class Device;

class EnBuffer : public Resource {
public:
	EnBuffer();
	static EnBuffer* Create(Device* device, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags flags);
	VkBuffer buffer;
	virtual void bindMemory(Device* device, VkDeviceMemory memory, uint64_t localOffset) override;
	virtual void destroy(Device* device) override;
	void* mapMemory(Device* device);
	void unMapMemory(Device* device);
	void setObjectName(Device* device, const char* name);
};

