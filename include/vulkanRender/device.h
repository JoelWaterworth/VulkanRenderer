#pragma once
#include <vulkan/vulkan.h>
#include "resources/resource.h"
#include <vector>
#include "glm/glm.hpp"
#include <tuple>

using namespace std;

struct Block {
	VkDeviceMemory allocation;
	uint64_t currentOffset;
	uint32_t typeBit;
};

class Device
{
public:
	Device(VkInstance instance, VkSurfaceKHR surface);
	Device(std::nullptr_t)
		: _d(nullptr)
	{}
	~Device();
	void deallocateAll();

	uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32 *memTypeFound = nullptr);

	std::pair<VkBuffer, VkDeviceMemory> allocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

	void attachResource(Resource * resource, VkMemoryPropertyFlags requirementsMask);
	
	inline VkQueue getGraphicsQueue() const	{ return graphicsQueue; };
	inline VkQueue getPresentQueue() const	{ return presentQueue; };
	inline VkDevice handle() const { return _d; };

	void createCommandBuffer(VkCommandBufferLevel level, uint8_t commandBufferNum, VkCommandBuffer* commandBuffers);
	void submitCommandBuffer(VkCommandBuffer cmd, bool free);

	void setUpmarkers();
	void setObjectName(uint64_t object, VkDebugReportObjectTypeEXT objectType, const char *name);
	void setSemaphoreName(VkSemaphore semaphore, const char *name);
	void setObjectTag(uint64_t object, VkDebugReportObjectTypeEXT objectType, uint64_t name, size_t tageSize, const void *tag);
	void beginRegion(VkCommandBuffer cmdbuffer, const char* pMarkerName, glm::vec4 color);
	void insert(VkCommandBuffer cmdbuffer, const char* pMarkerName, glm::vec4 color);
	void endRegion(VkCommandBuffer cmdbuffer);

private:
	Block* allocateBlock(uint32_t typeBit);

public:
	VkCommandPool _commandPool;
	uint32_t graphicsQueueFamilyIndex;
	uint32_t queueFamilyCount;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	VkPhysicalDeviceProperties _deviceProperties;
	VkPhysicalDevice _gpu = nullptr;
	bool debugMarkerActive = false;

private:
	VkDevice _d = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	vector<Block> allocations;
	uint64_t blockSize = 512;

	PFN_vkDebugMarkerSetObjectTagEXT     vkDebugMarkerSetObjectTag = VK_NULL_HANDLE;
	PFN_vkDebugMarkerSetObjectNameEXT	 vkDebugMarkerSetObjectName = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerBeginEXT		 cmdDebugMarkerBegin = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerEndEXT			 cmdDebugMarkerEnd = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerInsertEXT		 cmdDebugMarkerInsert = VK_NULL_HANDLE;
};

