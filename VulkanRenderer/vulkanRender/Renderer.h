#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>

#include "EnDevice.h"
#include "RenderTarget.h"
#include "resources\ResourceManger.h"
#include <string>
#include "resources\Mesh.h"
#include "Shader.h"
#include "UniformInterface.h"
#include "resources\Material.h"
#include "resources\Texture.h"
#include "resources\Resource.h"
#include "resources\ResourceManger.h"

#define FRAME_LAG 3
#ifndef VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NO_EXCEPTIONS
#endif // !VULKAN_HPP_NO_EXCEPTIONS


struct SurfaceCapabilities {
	vk::SurfaceCapabilitiesKHR capabilities;
	vk::SurfaceTransformFlagBitsKHR preTransform;
	uint32_t desiredImageCount;
	vk::SurfaceFormatKHR format;
};

class Window;

class Renderer
{
public:
	Renderer(std::string title, Window* window);
	~Renderer();

	void Run();
private:
	void initInstance(std::string title);
	void initDebug();
	void SetupDebugMarkers();
	void initDevice();
	void GetCapabilities();
	void CreateSwapchain();
	void CreateFencesSemaphore();
	void BuildPresentCommandBuffer(vk::CommandBuffer commandBuffer);
	void SetObjectName(uint64_t objectId, vk::DebugReportObjectTypeEXT objectType, const char *name);
	bool debugMarkerActive = false;
	bool prepared = false;
	vk::Queue presentQueue;
	vk::Queue graphicsQueue;
	vk::DebugReportCallbackCreateInfoEXT dbgCreateInfo = vk::DebugReportCallbackCreateInfoEXT();
	VkDebugReportCallbackEXT			 debugReport = VK_NULL_HANDLE;

	PFN_vkDebugMarkerSetObjectTagEXT     debugMarkerSetObjectTag = VK_NULL_HANDLE;
	PFN_vkDebugMarkerSetObjectNameEXT	 debugMarkerSetObjectName = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerBeginEXT		 cmdDebugMarkerBegin = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerEndEXT			 cmdDebugMarkerEnd = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerInsertEXT		 cmdDebugMarkerInsert = VK_NULL_HANDLE;

	std::vector<vk::CommandBuffer> _draw;
	vk::CommandPool _commandPool;

	vk::Semaphore _completeRender[FRAME_LAG];
	vk::Semaphore _presentComplete[FRAME_LAG];

	vk::Fence _fences[FRAME_LAG];
	uint8_t _frameIndex = 0;
	uint32_t currentBuffer = 0;
	uint32_t graphicsQueueFamilyIndex;
	ResourceManger* resourceManger;
	Mesh* _mesh = nullptr;
	Shader* _shader = nullptr;
	Material* _material = nullptr;
	UniformInterface* _unfirom = nullptr;
	vk::Instance instance = nullptr;
	EnDevice* _device = nullptr;
	SurfaceCapabilities capabilities;
	vk::SurfaceKHR surface = nullptr;
	vk::PhysicalDevice gpu = nullptr;
	uint32_t queueFamilyCount;
	RenderTarget* renderTarget = nullptr;
	struct {
		vk::SwapchainKHR handle;
		std::vector<vk::Image> images;
		std::vector<vk::ImageView> view;
	} swapchain;
};

