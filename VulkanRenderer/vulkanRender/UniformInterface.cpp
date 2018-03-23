#include "UniformInterface.h"

UniformInterface::~UniformInterface()
{
}

VkDescriptorType UniformInterface::getDescriptorType()
{
	return VkDescriptorType();
}

VkDescriptorBufferInfo* UniformInterface::getBufferInfo()
{
	return nullptr;
}

VkDescriptorImageInfo * UniformInterface::getImageInfo()
{
	return nullptr;
}
