#pragma once
#include <vulkan/vulkan.h>

class Device;

class Resource {
public:
	inline VkMemoryRequirements getRequirments() const { return requirments; };
	virtual void bindMemory(Device* device, VkDeviceMemory memory, uint64_t localOffset);
	virtual void destroy(Device* device);
	uint64_t _offset;
	VkDeviceSize size;
	VkDeviceMemory memory = nullptr;
protected:
	VkMemoryRequirements requirments;
};
