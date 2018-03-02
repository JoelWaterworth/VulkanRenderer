#pragma once
#include "Resource.h"
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include "../EnDevice.h"

class Uniform
{
public:
	virtual ~Uniform();
	virtual vk::DescriptorType getDescriptorType();
	virtual vk::DescriptorBufferInfo* getBufferInfo();
};

