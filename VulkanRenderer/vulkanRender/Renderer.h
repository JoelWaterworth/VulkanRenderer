#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>

#include "EnDevice.h"
#include "RenderTarget.h"
#include "reasources\ResourceManger.h"
#include <string>
#include "reasources\Mesh.h"
#include "reasources\Shader.h"

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
private:
	void initInstance(std::string title);
	void initDebug();
	void initDevice();
	void GetCapabilities();
	void CreateSwapchain();

	vk::DebugReportCallbackCreateInfoEXT dbgCreateInfo = vk::DebugReportCallbackCreateInfoEXT();
	VkDebugReportCallbackEXT			 debugReport = VK_NULL_HANDLE;

	ResourceManger* resourceManger;
	Mesh* mesh = nullptr;
	vk::Instance instance = nullptr;
	EnDevice device = nullptr;
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

