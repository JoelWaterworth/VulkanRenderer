#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include "resources\Resource.h"
#include "glm\glm.hpp"

using namespace std;

struct Block {
	vk::DeviceMemory allocation;
	uint64_t currentOffset;
	uint32_t typeBit;
};

class EnDevice : public vk::Device
{
public:
	EnDevice(vk::Instance instance, vk::SurfaceKHR surface);
	EnDevice(std::nullptr_t)
		: vk::Device(nullptr)
	{}
	void deallocateAll();
	vk::PhysicalDeviceMemoryProperties memoryProperties;

	bool memoryTypeFromProperties(vk::MemoryRequirements memReq, vk::MemoryPropertyFlags requirementsMask, uint32_t* typeIndex);
	int32_t findProperties(uint32_t memoryTypeBitsRequirement, vk::MemoryPropertyFlags requiredProperties);
	uint32_t getMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags properties, vk::Bool32 *memTypeFound = nullptr);

	std::pair<vk::Buffer, vk::DeviceMemory> allocateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

	void attachResource(Resource * resource, vk::MemoryPropertyFlags requirementsMask);
	bool debugMarkerActive = false;
	Block* allocateBlock(uint32_t typeBit);

	vk::CommandPool _commandPool;
	uint32_t graphicsQueueFamilyIndex;
	uint32_t queueFamilyCount;
	vk::PhysicalDevice _gpu = nullptr;
	inline vk::Queue getGraphicsQueue() const	{ return graphicsQueue; };
	inline vk::Queue getPresentQueue() const	{ return presentQueue; };

	void createCommandBuffer(vk::CommandBufferLevel level, uint8_t commandBufferNum, vk::CommandBuffer* commandBuffers);
	void submitCommandBuffer(vk::CommandBuffer cmd, bool free);

	void setUpmarkers();
	void setObjectName(uint64_t object, vk::DebugReportObjectTypeEXT objectType, const char *name);
	void setObjectTag(uint64_t object, vk::DebugReportObjectTypeEXT objectType, uint64_t name, size_t tageSize, const void *tag);
	void beginRegion(vk::CommandBuffer cmdbuffer, const char* pMarkerName, std::array<float, 4Ui64> color);
	void insert(vk::CommandBuffer cmdbuffer, const char* pMarkerName, std::array<float, 4Ui64> color);
	void endRegion(vk::CommandBuffer cmdbuffer);

private:

	vk::Queue presentQueue;
	vk::Queue graphicsQueue;
	vector<Block> allocations;
	uint64_t blockSize = 512;

	PFN_vkDebugMarkerSetObjectTagEXT     debugMarkerSetObjectTag = VK_NULL_HANDLE;
	PFN_vkDebugMarkerSetObjectNameEXT	 debugMarkerSetObjectName = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerBeginEXT		 cmdDebugMarkerBegin = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerEndEXT			 cmdDebugMarkerEnd = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerInsertEXT		 cmdDebugMarkerInsert = VK_NULL_HANDLE;
};

