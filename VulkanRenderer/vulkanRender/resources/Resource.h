#pragma once
#include "../EnDevice.h"

class Resource
{
public:
	inline vk::MemoryRequirements getRequirments() const { return requirments; };
	virtual void bindMemory(vk::DeviceMemory memory);
	virtual bool postAllocation();
	uint64_t _offset;
protected:
	EnDevice* _device;
	vk::MemoryRequirements requirments;
};