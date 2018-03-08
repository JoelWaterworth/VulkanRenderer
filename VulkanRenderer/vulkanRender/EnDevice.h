#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include "resources\Resource.h"

using namespace std;

struct Block {
	vk::DeviceMemory allocation;
	vector<Resource*> resources;
	uint64_t currentOffset;
};

class EnDevice : public vk::Device
{
public:
	EnDevice(std::nullptr_t)
		: vk::Device(nullptr)
	{}
	void deallocateAll();
	static EnDevice* Create(vk::PhysicalDevice gpu, vk::DeviceCreateInfo createInfo, const vk::AllocationCallbacks* pAllocator = NULL);
	vk::PhysicalDeviceMemoryProperties memoryProperties;

	bool memoryTypeFromProperties(vk::MemoryRequirements memReq, vk::MemoryPropertyFlags requirementsMask, uint32_t* typeIndex);

	std::pair<vk::Buffer, vk::DeviceMemory> allocateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

	void attachResource(Resource* resource, vk::MemoryPropertyFlags memoryFlags);

	void allocateBlock(vk::MemoryPropertyFlags memoryFlags);

	void allocate(const vector<Resource*>& resources, vk::MemoryPropertyFlags memoryFlags);
private:
	vector<Block> allocations;
	uint64_t blockSize = 512;
};

