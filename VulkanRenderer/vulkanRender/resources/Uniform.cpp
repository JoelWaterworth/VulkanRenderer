#include "Uniform.h"

Uniform::~Uniform()
{
}

vk::DescriptorType Uniform::getDescriptorType()
{
	return vk::DescriptorType();
}

vk::DescriptorBufferInfo* Uniform::getBufferInfo()
{
	return nullptr;
}
