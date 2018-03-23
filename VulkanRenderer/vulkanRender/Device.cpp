#include "Device.h"
#include <vulkan\vulkan.h>
#include <vulkan\vk_icd.h>
#include "util.h"
#include <iostream>
#include <functional>
#include "resources\Resource.h"

void Device::deallocateAll()
{
	for (auto allocation : allocations) {
		vkFreeMemory(_d, allocation.allocation, nullptr);
	}
	vkDestroyCommandPool(_d, _commandPool, nullptr);
}

uint32_t Device::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32 * memTypeFound)
{
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		if ((typeBits & 1) == 1) {
			if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				if (memTypeFound) {
					*memTypeFound = true;
				}
				return i;
			}
		}
		typeBits >>= 1;
	}

	if (memTypeFound) {
		*memTypeFound = false;
		return 0;
	} else {
		throw std::runtime_error("Could not find a matching memory type");
	}
}

std::pair<VkBuffer, VkDeviceMemory> Device::allocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkBuffer buffer;
	vkCreateBuffer(_d, &bufferInfo, nullptr, &buffer);
	VkMemoryRequirements req;
	vkGetBufferMemoryRequirements(_d, buffer, &req);

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = req.size;
	allocateInfo.memoryTypeIndex = this->getMemoryType(
		req.memoryTypeBits,
		properties,
		&allocateInfo.memoryTypeIndex);
	VkDeviceMemory memory;
	VK_CHECK_RESULT(vkAllocateMemory(_d, &allocateInfo, nullptr, &memory));
	vkBindBufferMemory(_d, buffer, memory, 0);
	return std::pair<VkBuffer, VkDeviceMemory>(buffer, memory);
}

void Device::attachResource(Resource * resource, VkMemoryPropertyFlags requirementsMask)
{
	uint32_t typeBit = getMemoryType(resource->getRequirments().memoryTypeBits, requirementsMask);

	for (auto& allocation : allocations) {
		if (allocation.typeBit == typeBit) {
			VkMemoryRequirements req = resource->getRequirments();
			resource->_offset = allocation.currentOffset + req.alignment - allocation.currentOffset % req.alignment;
			resource->bindMemory(this, allocation.allocation, 0);
			allocation.currentOffset += req.size;
			resource->memory = allocation.allocation;
			return;
		}
	}

	Block* b = allocateBlock(typeBit);
	resource->_offset = b->currentOffset;
	resource->bindMemory(this, b->allocation, 0);
	b->currentOffset += resource->getRequirments().size;
	resource->memory = b->allocation;
}

Block* Device::allocateBlock(uint32_t typeBit)
{
	VkMemoryAllocateInfo memoryInfo = {};
	memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryInfo.allocationSize = blockSize * 1024 * 1024;
	memoryInfo.memoryTypeIndex = typeBit;
	VkDeviceMemory memory;
	vkAllocateMemory(_d, &memoryInfo, nullptr, &memory);
	allocations.push_back({ memory, 0, typeBit });
	return &allocations[allocations.size() - 1];
}

void Device::createCommandBuffer(VkCommandBufferLevel level, uint8_t commandBufferNum, VkCommandBuffer* commandBuffers) {
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandPool = _commandPool;
	info.level = level;
	info.commandBufferCount = commandBufferNum;
	vkAllocateCommandBuffers(_d, &info, commandBuffers);
}

void Device::submitCommandBuffer(VkCommandBuffer cmd, bool free) {
	vkEndCommandBuffer(cmd);
	
	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.commandBufferCount = 1;
	info.pCommandBuffers = &cmd;
	vkQueueSubmit(graphicsQueue, 1, &info, nullptr);
	vkQueueWaitIdle(graphicsQueue);
}

void Device::setUpmarkers()
{
	bool debugExtensionPresent = false;
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(_gpu, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(_gpu, nullptr, &extensionCount, extensions.data());

	for (auto extension : extensions) {
		if (strcmp(extension.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0) {
			debugExtensionPresent = true;
			break;
		}
	}
	
	if (debugExtensionPresent) {
		debugMarkerSetObjectTag		= reinterpret_cast<PFN_vkDebugMarkerSetObjectTagEXT>(vkGetDeviceProcAddr(_d, "vkDebugMarkerSetObjectTagEXT"));
		debugMarkerSetObjectName	= reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(vkGetDeviceProcAddr(_d, "vkDebugMarkerSetObjectNameEXT"));
		cmdDebugMarkerBegin			= reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetDeviceProcAddr(_d, "vkCmdDebugMarkerBeginEXT"));
		cmdDebugMarkerEnd			= reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetDeviceProcAddr(_d, "vkCmdDebugMarkerEndEXT"));
		cmdDebugMarkerInsert		= reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>(vkGetDeviceProcAddr(_d, "vkCmdDebugMarkerInsertEXT"));
		debugMarkerActive = (debugMarkerSetObjectName != VK_NULL_HANDLE);
		if (!debugMarkerActive) {
			std::cout << "debugMarker not active" << std::endl;
		}
	}
	else {
		std::cout << "Warning: " << VK_EXT_DEBUG_MARKER_EXTENSION_NAME << " not present, debug markers are disabled." << std::endl;
	}
}

void Device::setObjectName(uint64_t object, VkDebugReportObjectTypeEXT objectType, const char * name)
{
	if (debugMarkerActive) {
		VkDebugMarkerObjectNameInfoEXT info = {};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
		info.objectType = objectType;
		info.pObjectName = name;
		info.object = object;
		debugMarkerSetObjectName(_d, &info);
		std::cout << name << std::endl;
	}
}

void Device::setSemaphoreName(VkSemaphore semaphore, const char * name)
{
	setObjectName((uint64_t)semaphore, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, name);
}

void Device::setObjectTag(uint64_t object, VkDebugReportObjectTypeEXT objectType, uint64_t name, size_t tageSize, const void * tag)
{
}

void Device::beginRegion(VkCommandBuffer cmdbuffer, const char * pMarkerName, glm::vec4 color) {
	// Check for valid function pointer (may not be present if not running in a debugging application)
	if (debugMarkerActive) {
		VkDebugMarkerMarkerInfoEXT info = {};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
		memcpy(info.color, &color[0], sizeof(float) * 4);
		info.pMarkerName = pMarkerName;
		cmdDebugMarkerBegin(cmdbuffer, &info);
		std::cout << pMarkerName << std::endl;
	}
}

void Device::insert(VkCommandBuffer cmdbuffer, const char * pMarkerName, glm::vec4 color)
{
}

void Device::endRegion(VkCommandBuffer cmdbuffer)
{
	// Check for valid function (may not be present if not runnin in a debugging application)
	if (debugMarkerActive)
	{
		cmdDebugMarkerEnd(cmdbuffer);
	}
}

Device::Device(VkInstance instance, VkSurfaceKHR surface)
{
	uint32_t gpu_count;
	vkEnumeratePhysicalDevices(instance, &gpu_count, NULL);
	std::vector<VkPhysicalDevice> physicalDevices(gpu_count);

	if (gpu_count > 0) {
		vkEnumeratePhysicalDevices(instance, &gpu_count, physicalDevices.data());
	}
	else {
		assert(0 && "no valid gpu");
	}

	_gpu = physicalDevices[0];

	vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &queueFamilyCount, NULL);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &queueFamilyCount, queueFamilyProperties.data());


	std::vector<VkBool32> supportPresents(queueFamilyCount);
	graphicsQueueFamilyIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		vkGetPhysicalDeviceSurfaceSupportKHR(_gpu, i, surface, &supportPresents[i]);
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && supportPresents[i] == VK_TRUE) {
			graphicsQueueFamilyIndex = i;
		}
	}

	if (graphicsQueueFamilyIndex == UINT32_MAX) {
		assert(0 && "Vulkan no valid graphics queue family index.");
	}

	float queue_priorities[1] = { 0.0 };
	VkDeviceQueueCreateInfo queueInfo = {};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = queue_priorities;

	std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pQueueCreateInfos = &queueInfo;
	createInfo.enabledExtensionCount = deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	VK_CHECK_RESULT(vkCreateDevice(_gpu, &createInfo, nullptr, &_d));
	vkGetPhysicalDeviceMemoryProperties(_gpu, &memoryProperties);
	
	vkGetDeviceQueue(_d, graphicsQueueFamilyIndex, 0, &graphicsQueue);
	presentQueue = graphicsQueue;

	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
	VK_CHECK_RESULT(vkCreateCommandPool(_d, &cmdPoolInfo, nullptr, &_commandPool));
	setUpmarkers();
}
Device::~Device() {
	vkDestroyDevice(_d, nullptr);
}