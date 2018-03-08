#include "ResourceManger.h"
#include <iostream>

ResourceManger::ResourceManger(EnDevice* device)
{
	_device = device;
}


ResourceManger::~ResourceManger()
{
	for (auto alloc : allocations) {
		_device->freeMemory(alloc.allocation);
	}
}

void ResourceManger::allocate(const vector<Resource*>& resources, vk::MemoryPropertyFlags memoryFlags)
{
	uint32_t size = 0;
	for (auto resource : resources) {
		resource->_offset = size;
		size += resource->getRequirments().size;
	}
	vk::MemoryRequirements memReq = resources[0]->getRequirments();

	uint32_t memoryIndex = 0;

	if (!_device->memoryTypeFromProperties(
		memReq,
		memoryFlags,
		&memoryIndex)) {
		assert("no suitable memory type");
	};

	auto const memoryInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(size)
		.setMemoryTypeIndex(memoryIndex);
	vk::DeviceMemory memory = _device->allocateMemory(memoryInfo);
	for (auto resource : resources) {
		resource->bindMemory(memory);
		resource->postAllocation();
	}
	allocations.push_back({ memory, resources });
}

Mesh* ResourceManger::getAsset(path path)
{
	return nullptr;
}
