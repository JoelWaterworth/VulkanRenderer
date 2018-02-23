#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>

class EnDevice : public vk::Device
{
public:
	EnDevice(std::nullptr_t)
		: vk::Device(nullptr)
	{}
	static EnDevice* Create(vk::PhysicalDevice gpu, vk::DeviceCreateInfo createInfo, const vk::AllocationCallbacks* pAllocator = NULL);
	vk::PhysicalDeviceMemoryProperties memoryProperties;

	bool memoryTypeFromProperties(vk::MemoryRequirements memReq, vk::MemoryPropertyFlags requirementsMask, uint32_t* typeIndex);

	std::pair<vk::Buffer, vk::DeviceMemory> allocateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlagBits properties);
private:
	
};

