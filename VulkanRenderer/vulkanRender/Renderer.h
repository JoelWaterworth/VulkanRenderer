#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>

#include "EnDevice.h"
#include "RenderTarget.h"
#include <string>
#include "resources\Mesh.h"
#include "Shader.h"
#include "UniformInterface.h"
#include "resources\Material.h"
#include "resources\Texture.h"
#include "resources\Resource.h"

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
	void initDevice();
	void GetCapabilities();
	void CreateSwapchain();
	void CreateFencesSemaphore();
	void BuildPresentCommandBuffer(vk::CommandBuffer commandBuffer);
	bool debugMarkerActive = false;
	bool prepared = false;
	vk::DebugReportCallbackCreateInfoEXT dbgCreateInfo = vk::DebugReportCallbackCreateInfoEXT();
	VkDebugReportCallbackEXT			 debugReport = VK_NULL_HANDLE;

	std::vector<vk::CommandBuffer> _draw;

	vk::Semaphore _completeRender[FRAME_LAG];
	vk::Semaphore _presentComplete[FRAME_LAG];

	vk::Fence _fences[FRAME_LAG];
	uint8_t _frameIndex = 0;
	uint32_t currentBuffer = 0;
	Texture* _texture = nullptr;
	Mesh* _mesh = nullptr;
	Shader* _presentShader = nullptr;
	Shader* _deferredShader = nullptr;
	Material* _material = nullptr;
	UniformInterface* _unfirom = nullptr;
	vk::Instance instance = nullptr;
	EnDevice* _device = nullptr;
	SurfaceCapabilities capabilities;
	vk::SurfaceKHR surface = nullptr;
	RenderTarget* PresentRenderTarget = nullptr;
	RenderTarget* DeferredRenderTarget = nullptr;
	struct {
		vk::SwapchainKHR handle;
		std::vector<vk::Image> images;
		std::vector<vk::ImageView> view;
	} swapchain;
};

