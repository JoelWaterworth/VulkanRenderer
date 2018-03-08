#pragma once
#include <vulkan/vulkan.hpp>

class EnDevice;

class Resource
{
public:
	inline vk::MemoryRequirements getRequirments() const { return requirments; };
	virtual void bindMemory(EnDevice* device, vk::DeviceMemory memory, uint64_t localOffset);
	virtual void destroy(EnDevice* device);
	uint64_t _offset;
protected:
	vk::MemoryRequirements requirments;
};