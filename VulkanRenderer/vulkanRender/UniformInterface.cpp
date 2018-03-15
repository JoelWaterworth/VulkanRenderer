#include "UniformInterface.h"

UniformInterface::~UniformInterface()
{
}

vk::DescriptorType UniformInterface::getDescriptorType()
{
	return vk::DescriptorType();
}

vk::DescriptorBufferInfo* UniformInterface::getBufferInfo()
{
	return nullptr;
}

vk::DescriptorImageInfo * UniformInterface::getImageInfo()
{
	return nullptr;
}
