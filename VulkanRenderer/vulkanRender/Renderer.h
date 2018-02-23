#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>

#include "EnDevice.h"
#include "RenderTarget.h"
#include "reasources\ResourceManger.h"
#include <string>
#include "reasources\Mesh.h"
#include "reasources\Shader.h"

#define FRAME_LAG 3

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

