#include "EnDevice.h"
#include <vulkan\vulkan.h>
#include <vulkan\vk_icd.h>
#include "util.h"
#include <iostream>
#include <functional>
#include "resources\Resource.h"

void EnDevice::deallocateAll()
{
	for (auto allocation : allocations) {
		freeMemory(allocation.allocation);
	}
	destroyCommandPool(_commandPool);
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

int32_t EnDevice::findProperties( uint32_t memoryTypeBitsRequirement, vk::MemoryPropertyFlags requiredProperties) {
	const uint32_t memoryCount = memoryProperties.memoryTypeCount;
	for (uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex) {
		const uint32_t memoryTypeBits = (1 << memoryIndex);
		const bool isRequiredMemoryType = memoryTypeBitsRequirement & memoryTypeBits;

		const vk::MemoryPropertyFlags properties =
			memoryProperties.memoryTypes[memoryIndex].propertyFlags;
		const bool hasRequiredProperties =
			(properties & requiredProperties) == requiredProperties;

		if (isRequiredMemoryType && hasRequiredProperties)
			return static_cast<int32_t>(memoryIndex);
	}

	// failed to find memory type
	return -1;
}

uint32_t EnDevice::getMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags properties, vk::Bool32 * memTypeFound)
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

void EnDevice::attachResource(Resource * resource, vk::MemoryPropertyFlags requirementsMask)
{
	uint32_t typeBit = getMemoryType(resource->getRequirments().memoryTypeBits, requirementsMask);

	for (auto& allocation : allocations) {
		if (allocation.typeBit == typeBit) {
			vk::MemoryRequirements req = resource->getRequirments();
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

Block* EnDevice::allocateBlock(uint32_t typeBit)
{
	auto const memoryInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(blockSize * 1024 * 1024)
		.setMemoryTypeIndex(typeBit);
	vk::DeviceMemory memory = allocateMemory(memoryInfo);
	allocations.push_back({ memory, 0, typeBit });
	return &allocations[allocations.size() - 1];
}

void EnDevice::createCommandBuffer(vk::CommandBufferLevel level, uint8_t commandBufferNum, vk::CommandBuffer* commandBuffers) {
	auto const info = vk::CommandBufferAllocateInfo(_commandPool, level, commandBufferNum);
	allocateCommandBuffers(&info, commandBuffers);
}

void EnDevice::submitCommandBuffer(vk::CommandBuffer cmd, bool free) {
	cmd.end();

	auto const info = vk::SubmitInfo()
		.setCommandBufferCount(1)
		.setPCommandBuffers(&cmd);
	graphicsQueue.submit(1, &info, nullptr);
	graphicsQueue.waitIdle();
}

void EnDevice::setUpmarkers()
{
	bool debugExtensionPresent = false;
	uint32_t extensionCount = 0;
	_gpu.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<vk::ExtensionProperties> extensions(extensionCount);
	_gpu.enumerateDeviceExtensionProperties(nullptr, &extensionCount, extensions.data());

	for (auto extension : extensions) {
		if (strcmp(extension.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0) {
			debugExtensionPresent = true;
			break;
		}
	}
	
	if (debugExtensionPresent) {
		debugMarkerSetObjectTag		= reinterpret_cast<PFN_vkDebugMarkerSetObjectTagEXT>(getProcAddr("vkDebugMarkerSetObjectTagEXT"));
		debugMarkerSetObjectName	= reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(getProcAddr("vkDebugMarkerSetObjectNameEXT"));
		cmdDebugMarkerBegin			= reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(getProcAddr("vkCmdDebugMarkerBeginEXT"));
		cmdDebugMarkerEnd			= reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(getProcAddr("vkCmdDebugMarkerEndEXT"));
		cmdDebugMarkerInsert		= reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>(getProcAddr("vkCmdDebugMarkerInsertEXT"));
		debugMarkerActive = (debugMarkerSetObjectName != VK_NULL_HANDLE);
		debugMarkerActive = false;
		if (!debugMarkerActive) {
			std::cout << "debugMarker not active" << std::endl;
		}
	}
	else {
		std::cout << "Warning: " << VK_EXT_DEBUG_MARKER_EXTENSION_NAME << " not present, debug markers are disabled." << std::endl;
	}
}

void EnDevice::setObjectName(uint64_t object, vk::DebugReportObjectTypeEXT objectType, const char * name)
{
	if (debugMarkerActive) {
		auto const info = vk::DebugMarkerObjectNameInfoEXT()
			.setObjectType(objectType)
			.setPObjectName(name)
			.setObject(object);
		VkDevice d = *reinterpret_cast<VkDevice*>(this);
		debugMarkerSetObjectName(d, reinterpret_cast<const VkDebugMarkerObjectNameInfoEXT*>(&info));
		std::cout << name << std::endl;
	}
}

void EnDevice::setSemaphoreName(vk::Semaphore semaphore, const char * name)
{
	VkSemaphore s = *reinterpret_cast<VkSemaphore*>(&semaphore);
	//setObjectName((uint64_t)s, vk::DebugReportObjectTypeEXT::eSemaphore, name);
}

void EnDevice::setObjectTag(uint64_t object, vk::DebugReportObjectTypeEXT objectType, uint64_t name, size_t tageSize, const void * tag)
{
}

void EnDevice::beginRegion(vk::CommandBuffer cmdbuffer, const char * pMarkerName, std::array<float, 4Ui64> color)
{
	// Check for valid function pointer (may not be present if not running in a debugging application)
	if (debugMarkerActive)
	{
		auto const info = vk::DebugMarkerMarkerInfoEXT()
			.setColor(color)
			.setPMarkerName(pMarkerName);
		cmdDebugMarkerBegin(cmdbuffer, reinterpret_cast<const VkDebugMarkerMarkerInfoEXT*>(&info));
		std::cout << pMarkerName << std::endl;
	}
}

void EnDevice::insert(vk::CommandBuffer cmdbuffer, const char * pMarkerName, std::array<float, 4Ui64> color)
{
}

void EnDevice::endRegion(vk::CommandBuffer cmdbuffer)
{
	// Check for valid function (may not be present if not runnin in a debugging application)
	if (debugMarkerActive)
	{
		cmdDebugMarkerEnd(cmdbuffer);
	}
}

EnDevice::EnDevice(vk::Instance instance, vk::SurfaceKHR surface)
{
	uint32_t gpu_count;
	instance.enumeratePhysicalDevices(&gpu_count, NULL);
	std::vector<vk::PhysicalDevice> physicalDevices(gpu_count);

	if (gpu_count > 0) {
		instance.enumeratePhysicalDevices(&gpu_count, physicalDevices.data());
	}
	else {
		assert(0 && "no valid gpu");
	}

	_gpu = physicalDevices[0];


	_gpu.getQueueFamilyProperties(&queueFamilyCount, NULL);
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	_gpu.getQueueFamilyProperties(&queueFamilyCount, queueFamilyProperties.data());


	std::vector<vk::Bool32> supportPresents(queueFamilyCount);
	graphicsQueueFamilyIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		_gpu.getSurfaceSupportKHR(i, surface, &supportPresents[i]);
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics && supportPresents[i] == VK_TRUE) {
			graphicsQueueFamilyIndex = i;
		}
	}

	if (graphicsQueueFamilyIndex == UINT32_MAX) {
		assert(0 && "Vulkan no valid graphics queue family index.");
	}

	float queue_priorities[1] = { 0.0 };
	auto const queueInfo = vk::DeviceQueueCreateInfo()
		.setQueueFamilyIndex(graphicsQueueFamilyIndex)
		.setQueueCount(1)
		.setPQueuePriorities(queue_priorities);

	std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	auto const createInfo = vk::DeviceCreateInfo()
		.setQueueCreateInfoCount(1)
		.setPQueueCreateInfos(&queueInfo)
		.setEnabledExtensionCount(deviceExtensions.size())
		.setPpEnabledExtensionNames(deviceExtensions.data());

	VK_CHECK_RESULT(_gpu.createDevice(&createInfo, nullptr, this));
	memoryProperties = _gpu.getMemoryProperties();
	getQueue(graphicsQueueFamilyIndex, 0, &graphicsQueue);
	presentQueue = graphicsQueue;

	auto const cmdPoolInfo = vk::CommandPoolCreateInfo().setQueueFamilyIndex(graphicsQueueFamilyIndex);
	VK_CHECK_RESULT(createCommandPool(&cmdPoolInfo, nullptr, &_commandPool));
	setUpmarkers();
}
