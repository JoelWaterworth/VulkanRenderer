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
#include "resources/shaderDescriptor.h"
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

	Texture _environmentCube;
	Texture _lutBrdf;
	Texture _irradianceCube;
	Texture _prefilteredCube;

	Texture _baseColour;
	Texture _normal;
	Texture _roughness;
	Texture _metallic;
	Texture _ao;

	Mesh _monkey;
	Mesh _plane;
	Mesh _box;
	 
	Shader _presentShader;
	Shader _deferredShader;
	Shader _skyboxShader;

	ShaderDescriptor _cameraDescriptor;
	ShaderDescriptor _positions;
	ShaderDescriptor _skyboxDescriptor;
	ShaderDescriptor _presentShaderDescriptor;
	ShaderDescriptor _deferredShaderDescriptor;

	UniformInterface* _unfirom = nullptr;
	UniformBuffer _cameraSpace;
	UniformDynamicBuffer _matPostion;
	UniformBuffer _lights;

	VkInstance instance = nullptr;
	Device* _device = nullptr;
	SurfaceCapabilities capabilities;
	CameraMat _cameraMat = {};
	VkSurfaceKHR surface = nullptr;
	RenderTarget PresentRenderTarget;
	RenderTarget DeferredRenderTarget;
	struct {
		VkSwapchainKHR handle;
		std::vector<VkImage> images;
		std::vector<VkImageView> view;
	} swapchain;
};

