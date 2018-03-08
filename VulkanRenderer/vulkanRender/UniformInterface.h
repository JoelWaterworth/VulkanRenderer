#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include "EnDevice.h"

class UniformInterface
{
public:
	virtual ~UniformInterface();
	virtual vk::DescriptorType getDescriptorType();
	virtual vk::DescriptorBufferInfo* getBufferInfo();
};

