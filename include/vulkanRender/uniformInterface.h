#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include "device.h"

class UniformInterface
{
public:
	virtual ~UniformInterface();
	virtual VkDescriptorType getDescriptorType();
	virtual VkDescriptorBufferInfo* getBufferInfo();
	virtual VkDescriptorImageInfo* getImageInfo();
};

