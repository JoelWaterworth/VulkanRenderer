#include "EnDevice.h"
#include "util.h"
#include <iostream>
#include <functional>
#include "resources\Resource.h"

void EnDevice::deallocateAll()
{
	for (auto allocation : allocations) {
		for (int i = 0; i < allocation.resources.size(); i++) {
			allocation.resources[i]->destroy(this);
		}
		freeMemory(allocation.allocation);
	}
}

EnDevice * EnDevice::Create(vk::PhysicalDevice gpu, vk::DeviceCreateInfo createInfo, const vk::AllocationCallbacks * pAllocator)
{
	EnDevice* device = new EnDevice(nullptr);
	VK_CHECK_RESULT(gpu.createDevice(&createInfo, pAllocator, device));
	device->allocateBlock(vk::MemoryPropertyFlagBits::eDeviceLocal);
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

void EnDevice::attachResource(Resource * resource, vk::MemoryPropertyFlags memoryFlags)
{
	Block* b = &allocations[allocations.size() - 1];
	resource->_offset = b->currentOffset;
	resource->bindMemory(this, b->allocation, 0);
	b->resources.push_back(resource);
	b->currentOffset += resource->getRequirments().size;
}

void EnDevice::allocateBlock(vk::MemoryPropertyFlags memoryFlags)
{
	auto const memoryInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(blockSize * 1024 * 1024)
		.setMemoryTypeIndex(7);
	vk::DeviceMemory memory = allocateMemory(memoryInfo);
	allocations.push_back({ memory, vector<Resource*>(), 0 });
}

void EnDevice::allocate(const vector<Resource*>& resources, vk::MemoryPropertyFlags memoryFlags)
{
	uint32_t size = 0;
	for (auto resource : resources) {
		resource->_offset = size;
		size += resource->getRequirments().size;
	}
	vk::MemoryRequirements memReq = resources[0]->getRequirments();

	uint32_t memoryIndex = 0;

	if (!memoryTypeFromProperties(
		memReq,
		memoryFlags,
		&memoryIndex)) {
		assert("no suitable memory type");
	};

	auto const memoryInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(size)
		.setMemoryTypeIndex(memoryIndex);
	vk::DeviceMemory memory = allocateMemory(memoryInfo);
	for (auto resource : resources) {
		resource->bindMemory(this, memory, 0);
		resource->postAllocation(this);
	}
	//allocations.push_back({ memory, resources });
}
