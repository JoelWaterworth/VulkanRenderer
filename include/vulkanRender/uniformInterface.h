#pragma once
#include <vulkan/vulkan.h>
#include "device.h"

class UniformInterface {
public:
	virtual ~UniformInterface();
	virtual VkDescriptorType getDescriptorType();
	virtual VkDescriptorBufferInfo* getBufferInfo();
	virtual VkDescriptorImageInfo* getImageInfo();
};

