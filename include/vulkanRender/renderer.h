#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>
#include "world.h"
#include "device.h"
#include "renderTarget.h"
#include <string>
#include "resources/mesh.h"
#include "shader.h"
#include "uniformInterface.h"
#include "resources/material.h"
#include "resources/texture.h"
#include "resources/resource.h"
#include "resources/uniformBuffer.h"
#include "resources/UniformDynamicBuffer.h"

#define FRAME_LAG 3
#ifndef VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NO_EXCEPTIONS
#endif // !VULKAN_HPP_NO_EXCEPTIONS

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
#define VULKAN_HPP_DISABLE_ENHANCED_MODE
#endif // !VULKAN_HPP_DISABLE_ENHANCED_MODE

struct UBO {
	Light light[4];
	glm::vec3 viewPos;
	int lightCount;
	UBO() : viewPos(glm::vec3(0.0f, 0.0f, 0.0f)), lightCount(0) {};
};

struct CameraMat {
	glm::mat4 per;
	glm::mat4 view;
	glm::mat4 model;
	Light light[4];
	glm::vec3 viewPos;
	int lightCount;
	float gamma;
	float exposure;
};


struct Model {
	glm::mat4 transform;
	glm::vec3 colour;
	float roughness;
	float metalic;
};

struct SurfaceCapabilities {
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceTransformFlagBitsKHR preTransform;
	uint32_t desiredImageCount;
	VkSurfaceFormatKHR format;
};

class WindowHandle;

class Renderer
{
public:
	Renderer();
	Renderer(std::string title, WindowHandle* window, bool bwValidation, bool bwDebugReport);
	~Renderer();

	void Run(World* world);
	void destroy();
private:
	void initInstance(std::string title, bool bwValidation, bool bwDebugReport);
	void initDebug();
	void initDevice();
	void GetCapabilities();
	void CreateSwapchain();
	void CreateFencesSemaphore();
	void BuildPresentCommandBuffer(VkCommandBuffer commandBuffer, World* world);
	void BuildOffscreenCommandBuffer(VkCommandBuffer cmd, World* world);
	void generateBRDFLUT();
	void generateIrradianceCube();
	void generatePrefilteredCube();
	bool debugMarkerActive = false;
	bool prepared = false;
	VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
	VkDebugReportCallbackEXT			 debugReport = VK_NULL_HANDLE;

	std::vector<VkCommandBuffer> _draw;
	VkCommandBuffer _offscreenDraw[FRAME_LAG];

	VkSemaphore _completeRender[FRAME_LAG];
	VkSemaphore _offscreenRender[FRAME_LAG];
	VkSemaphore _presentComplete[FRAME_LAG];

	VkFence _fences[FRAME_LAG];
	VkFence _renderFences[FRAME_LAG];
	uint8_t _frameIndex = 0;
	uint32_t currentBuffer = 0;

	Texture* _environmentCube = nullptr;
	Texture* _lutBrdf = nullptr;
	Texture* _irradianceCube = nullptr;
	Texture* _prefilteredCube = nullptr;

	Texture* _baseColour = nullptr;
	Texture* _normal = nullptr;
	Texture* _roughness = nullptr;
	Texture* _metallic = nullptr;
	Texture* _ao = nullptr;

	Mesh* _monkey = nullptr;
	Mesh* _plane = nullptr;
	Mesh* _box = nullptr;
	 
	Shader* _presentShader = nullptr;
	Shader* _deferredShader = nullptr;
	Shader* _skyboxShader = nullptr;
	Material _cameraDescriptor;
	Material _positions;
	Material _skyboxDescriptor;
	Material _presentMaterial;
	Material _deferredMaterial;
	UniformInterface* _unfirom = nullptr;
	UniformBuffer* _cameraSpace;
	UniformDynamicBuffer _matPostion;
	UniformBuffer* _lights = nullptr;
	VkInstance instance = nullptr;
	Device* _device = nullptr;
	SurfaceCapabilities capabilities;
	CameraMat _cameraMat = {};
	VkSurfaceKHR surface = nullptr;
	RenderTarget* PresentRenderTarget = nullptr;
	RenderTarget* DeferredRenderTarget = nullptr;
	struct {
		VkSwapchainKHR handle;
		std::vector<VkImage> images;
		std::vector<VkImageView> view;
	} swapchain;
};

