#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>

#include "EnDevice.h"
#include "RenderTarget.h"
#include "resources\ResourceManger.h"
#include <string>
#include "resources\Mesh.h"
#include "resources\Shader.h"
#include "resources\uniform.h"
#include "resources\Material.h"

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
	bool prepared = false;
	void initInstance(std::string title);
	void initDebug();
	void initDevice();
	void GetCapabilities();
	void CreateSwapchain();
	void CreateFencesSemaphore();
	void BuildPresentCommandBuffer(vk::CommandBuffer commandBuffer);
	vk::Queue presentQueue;
	vk::Queue graphicsQueue;
	vk::DebugReportCallbackCreateInfoEXT dbgCreateInfo = vk::DebugReportCallbackCreateInfoEXT();
	VkDebugReportCallbackEXT			 debugReport = VK_NULL_HANDLE;
	PFN_vkDebugMarkerSetObjectTagEXT     DebugMarkerSetObjectTag = VK_NULL_HANDLE;
	PFN_vkDebugMarkerSetObjectNameEXT	 DebugMarkerSetObjectName = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerBeginEXT		 CmdDebugMarkerBegin = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerEndEXT			 CmdDebugMarkerEnd = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerInsertEXT		 CmdDebugMarkerInsert = VK_NULL_HANDLE;

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
	Uniform* _unfirom = nullptr;
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

