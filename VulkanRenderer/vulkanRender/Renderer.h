#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>

#include "Device.h"
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

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
#define VULKAN_HPP_DISABLE_ENHANCED_MODE
#endif // !VULKAN_HPP_DISABLE_ENHANCED_MODE

struct SurfaceCapabilities {
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceTransformFlagBitsKHR preTransform;
	uint32_t desiredImageCount;
	VkSurfaceFormatKHR format;
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
	void BuildPresentCommandBuffer(VkCommandBuffer commandBuffer);
	void BuildOffscreenCommandBuffer();
	bool debugMarkerActive = false;
	bool prepared = false;
	VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
	VkDebugReportCallbackEXT			 debugReport = VK_NULL_HANDLE;

	std::vector<VkCommandBuffer> _draw;
	VkCommandBuffer _offscreenDraw;

	VkSemaphore _completeRender[FRAME_LAG];
	VkSemaphore _offscreenRender;
	VkSemaphore _presentComplete[FRAME_LAG];

	VkFence _fences[FRAME_LAG];
	uint8_t _frameIndex = 0;
	uint32_t currentBuffer = 0;
	Texture* _texture = nullptr;
	Mesh* _monkey = nullptr;
	Mesh* _plane = nullptr;
	Shader* _presentShader = nullptr;
	Shader* _deferredShader = nullptr;
	Material* _presentMaterial = nullptr;
	Material* _deferredMaterial = nullptr;
	UniformInterface* _unfirom = nullptr;
	VkInstance instance = nullptr;
	Device* _device = nullptr;
	SurfaceCapabilities capabilities;
	VkSurfaceKHR surface = nullptr;
	RenderTarget* PresentRenderTarget = nullptr;
	RenderTarget* DeferredRenderTarget = nullptr;
	struct {
		VkSwapchainKHR handle;
		std::vector<VkImage> images;
		std::vector<VkImageView> view;
	} swapchain;
};

