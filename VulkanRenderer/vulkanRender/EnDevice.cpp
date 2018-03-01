#include "EnDevice.h"
#include "util.h"
#include <iostream>
#include <functional>

EnDevice * EnDevice::Create(vk::PhysicalDevice gpu, vk::DeviceCreateInfo createInfo, const vk::AllocationCallbacks * pAllocator)
{
	EnDevice* device = new EnDevice(nullptr);
	VK_CHECK_RESULT(gpu.createDevice(&createInfo, pAllocator, device));
	return device;
}

bool EnDevice::memoryTypeFromProperties(vk::MemoryRequirements memReq, vk::MemoryPropertyFlags requirementsMask, uint32_t* typeIndex)
{
	auto findMemoryTypeIndex = [](EnDevice* device,vk::MemoryRequirements memReq, vk::MemoryPropertyFlags requirementsMask, uint32_t* typeIndex, std::function<bool(vk::MemoryPropertyFlags, vk::MemoryPropertyFlags)>) -> bool {
		uint32_t typeBits = memReq.memoryTypeBits;
		for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
			if ((typeBits & 1) == 1) {
				// Type is available, does it match user properties?
				if ((device->memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask) {
					*typeIndex = i;
					return true;
				}
			}
			typeBits >>= 1;
		}
		return false;
	};
	uint32_t ti = *typeIndex;
	if (findMemoryTypeIndex(this, memReq, requirementsMask, &ti, [](vk::MemoryPropertyFlags propertyFlags, vk::MemoryPropertyFlags flags) {
		return propertyFlags == flags;
	})) {
		*typeIndex = ti;
		return true;
	} 

	return findMemoryTypeIndex(this, memReq, requirementsMask, typeIndex, [](vk::MemoryPropertyFlags propertyFlags, vk::MemoryPropertyFlags flags) {
		return (propertyFlags & flags) == flags;
	});
}

std::pair<vk::Buffer, vk::DeviceMemory> EnDevice::allocateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
	auto const buffer_info = vk::BufferCreateInfo()
        .setSize(size)
        .setUsage(usage)
        .setSharingMode(vk::SharingMode::eExclusive);

	auto buffer = this->createBuffer(buffer_info);
	auto req = this->getBufferMemoryRequirements(buffer);
	auto allocateInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(req.size)
		.setMemoryTypeIndex(0);
	if (!this->memoryTypeFromProperties(
		req,
		properties,
		&allocateInfo.memoryTypeIndex)) {
		assert("no suitable memory type");
	};
	vk::DeviceMemory memory;
	VK_CHECK_RESULT(this->allocateMemory(&allocateInfo, nullptr, &memory));
	this->bindBufferMemory(buffer, memory, 0);
	return std::pair<vk::Buffer, vk::DeviceMemory>(buffer, memory);
}
