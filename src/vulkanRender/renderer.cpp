#include "renderer.h"
#include <vector>
#include <iostream>
#include <sstream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../windowHandler.h"
#include "util.h"
#include <vulkan/vulkan.h>
#include "lightActor.h"
#include <array>

#define M_PI       3.14159265358979323846 

PFN_vkCreateDebugReportCallbackEXT		CreateDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT		DestroyDebugReportCallbackEXT = VK_NULL_HANDLE;

VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject,
	size_t location,
	int32_t msgCode,
	const char *pLayerPrefix,
	const char *pMsg,
	void *pUserData) {
	// Select prefix depending on flags passed to the callback
	// Note that multiple flags may be set for a single validation message
	std::ostringstream stream;
	stream << "VKDBG: ";
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		stream << "INFO: ";
	}
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		stream << "WARNING: ";
	}
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		stream << "PERFORMANCE: ";
	}
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		stream << "ERROR: ";
	}
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		stream << "DEBUG: ";
	}
	stream << "@[" << pLayerPrefix << "]: ";
	stream << pMsg << std::endl;
	std::cout << stream.str();

#ifdef _WIN32
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		MessageBox(NULL, stream.str().c_str(), "Vulkan Error!", 0);
	}
#endif

	return false;
}

Renderer::Renderer() {
}

Renderer::Renderer(std::string title, WindowHandle* window, bool bwValidation, bool bwDebugReport) {
	initInstance(title, bwValidation, bwDebugReport);
	printf("initInstance\n");
	surface = window->createSurface(instance);
	printf("createSurface\n");
	initDevice();
	printf("initDevice\n");
	GetCapabilities();
	CreateSwapchain();
	printf("CreateSwapchain\n");
	std::array<AttachmentInfo, 7> defferedAttachmentInfo{ {
		{ VK_FORMAT_R8G8B8A8_UNORM,			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1 },
		{ VK_FORMAT_R16G16B16A16_SFLOAT,	VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1 },
		{ VK_FORMAT_R16G16B16A16_SFLOAT,	VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1 },
		{ VK_FORMAT_R8_UNORM,				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1 },
		{ VK_FORMAT_R8_UNORM,				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1 },
		{ VK_FORMAT_R8_UNORM,				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1 },
		{ VK_FORMAT_D16_UNORM,			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1 }
	} };

	AttachmentInfo PresentAttachmentInfo[] = {
		{ capabilities.format.format,		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1 },
		{ VK_FORMAT_D16_UNORM,			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,	VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1 } };


	VkExtent2D resolution = capabilities.capabilities.maxImageExtent;

	PresentRenderTarget = RenderTarget::Create(_device, resolution, PresentAttachmentInfo, 2, &swapchain.view);
	//DeferredRenderTarget = RenderTarget::Create(_device, resolution, defferedAttachmentInfo.data(), defferedAttachmentInfo.size());
  printf("PresentRenderTarget\n");
	_baseColour = Texture::Create(_device, path("assets/textures/Metal_basecolor.png"));
	_normal = Texture::Create(_device, path("assets/textures/Metal_normal.png"));
	_roughness = Texture::Create(_device, path("assets/textures/Metal_roughness.png"));
	_metallic = Texture::Create(_device, path("assets/textures/Metal_metallic.png"));
	_ao = Texture::Create(_device, path("assets/textures/Metal_AO.png"));

	_plane = Mesh::Create(_device, path("assets/mesh/plane.dae"));
	_plane.setBufferName(_device, "plane");
	_monkey = Mesh::Create(_device, path("assets/mesh/sphere.dae"));
	_box = Mesh::Create(_device, path("assets/mesh/cube.obj"));
	_environmentCube = Texture::CreateCubeMap(_device, path("assets/textures/hdr/pisa_cube.ktx"), VK_FORMAT_R16G16B16A16_SFLOAT);
	printf("load mesh complete\n");
	_monkey.setBufferName(_device, "monkey");
	/*
	std::vector<ShaderLayout> deferredLayout =
	{	ShaderLayout(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			VK_SHADER_STAGE_VERTEX_BIT,		0, 0),
		ShaderLayout(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	VK_SHADER_STAGE_VERTEX_BIT,		0, 1),
		ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,	0, 2),
		ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,	1, 2),
		ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,	2, 2),
		ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,	3, 2),
		ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,	4, 2)
	};
	*/
	std::vector<ShaderLayout> presentLayout = {
		ShaderLayout(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0, 0),

		ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 0),
		ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2, 0),
		ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3, 0),

		ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4, 0),//base color
		ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 5, 0),//normal
		ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 6, 0),//roughness
		ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 7, 0),//metallic
		ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 8, 0),//ao

		ShaderLayout(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0, 1)
	};

	std::vector<ShaderLayout> skyboxLayout = {
		ShaderLayout(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0, 0),
		ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 0),
	};

	printf("Create Shader\n");
	_presentShader =
	  Shader::Create(
	    _device,
	    &PresentRenderTarget,
	    path("assets/shaders/present.vert"),
	    path("assets/shaders/present.frag"),
	    presentLayout);
	_skyboxShader  =
	  Shader::Create(
	    _device,
	    &PresentRenderTarget,
	    path("assets/shaders/skybox.vert"),
	    path("assets/shaders/skybox.frag"),
	    skyboxLayout);
	//printf("Create _deferredShader\n");
	//_deferredShader = Shader::Create(_device, DeferredRenderTarget, path("assets/shaders/deferred.vert"), path("assets/shaders/deferred.frag"), deferredLayout);

	//_skyShader = Shader::Create(_device, PresentRenderTarget, path("assets/shaders/skybox.vert"), path("assets/shaders/skybox.frag"), presentLayout);

	_cameraMat = {};
	_cameraMat.per =
	  glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f) *
	  glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -5.0f));

	float spacing = 2.0f;
	vector<Model> models;
	for (int x = 0; x < 5; x++) {
		for (int y = 0; y < 5; y++) {
			Model m = {};
			m.transform = 
			  glm::translate(glm::mat4(1.0f), glm::vec3((x * spacing) - 4.0f, (y * spacing) - 4.0f, 5.0f)) *
			  glm::rotate(glm::mat4(1.0f), glm::radians(-0.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
			  glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
			m.metalic = y * 0.2f;
			m.roughness = x * 0.2f;
			m.colour = glm::vec3(1.0f, 0.0f, 0.0f);
			models.push_back(m);
		}
	}

	printf("Create _cameraSpace\n");
	_cameraSpace = UniformBuffer::CreateUniformBuffer(_device, _cameraMat);
	_matPostion = UniformDynamicBuffer::Create(_device, models);

	generateBRDFLUT();
	generateIrradianceCube();
	generatePrefilteredCube();

	std::vector<UniformBinding> cameraUniforms = { 
		UniformBinding(&_cameraSpace,  0, 0),
		UniformBinding(&_irradianceCube,  1, 0),
		UniformBinding(&_lutBrdf,  2, 0),
		UniformBinding(&_prefilteredCube,  3, 0),

		UniformBinding(&_baseColour,  4, 0),
		UniformBinding(&_normal,  5, 0),
		UniformBinding(&_roughness,  6, 0),
		UniformBinding(&_metallic,  7, 0),
		UniformBinding(&_ao,  8, 0)
	};
	std::vector<UniformBinding> posUniforms = { UniformBinding(&_matPostion,  0, 1)};
	/*
	std::vector<UniformBinding> deferredUniforms = { 
		UniformBinding(&_baseColour, 0, 2),
		UniformBinding(&_normal, 1, 2),
		UniformBinding(&_roughness, 2, 2),
		UniformBinding(&_metallic, 3, 2),
		UniformBinding(&_ao, 4, 2)
	};
	*/
	printf("Create _presentShaderDescriptor\n");

	UBO lights = {};
	lights.viewPos = glm::vec3(0.0f, 0.0f, -2.0f);
	lights.lightCount = 4;
	lights.light[0].radius = 4.0f;
	lights.light[0].color = glm::vec3(300.0f, 300.0f, 300.0f);
	lights.light[0].position = glm::vec4(10.0f, 10.0f, -10.0f, 1.0f);

	lights.light[1].radius = 4.0f;
	lights.light[1].color = glm::vec3(300.0f, 300.0f, 300.0f);
	lights.light[1].position = glm::vec4(-10.0f, 10.0f, -10.0f, 1.0f);

	lights.light[2].radius = 4.0f;
	lights.light[2].color = glm::vec3(300.0f, 300.0f, 300.0f);
	lights.light[2].position = glm::vec4(-10.0f, -10.0f, -10.0f, 1.0f);

	lights.light[3].radius = 4.0f;
	lights.light[3].color = glm::vec3(300.0f, 300.0f, 300.0f);
	lights.light[3].position = glm::vec4(10.0f, -10.0f, -10.0f, 1.0f);

	printf("Create _lights\n");
	_lights = UniformBuffer::CreateUniformBuffer(_device, lights);

	std::vector<UniformBinding> skyUniform = {
		UniformBinding(&_cameraSpace,  0, 0),
		UniformBinding(&_environmentCube,  1, 0),
	};

	//_presentShaderDescriptor = ShaderDescriptor(_device, _presentShader, _presentUniforms);
	printf("Create _presentShaderDescriptor\n");
	_cameraDescriptor = ShaderDescriptor(_device, &_presentShader, cameraUniforms, 0, false);
	_positions = ShaderDescriptor(_device, &_presentShader, posUniforms, 1, false, _matPostion.getAlign());
	_skyboxDescriptor = ShaderDescriptor(_device, &_skyboxShader, skyUniform, 0, false);
	//_deferredShaderDescriptor = ShaderDescriptor(_device, _deferredShader, deferredUniforms, 2);
	printf("begin CreateFencesSemaphores\n");
	CreateFencesSemaphore();

	prepared = true;
}

Renderer::~Renderer() {
	
}

void Renderer::Run(World* world) {
	if (!prepared) {
		return;
	}
	VK_CHECK_RESULT(vkWaitForFences(_device->handle(), 1, &_fences[_frameIndex], VK_TRUE, UINT64_MAX));
	VK_CHECK_RESULT(vkResetFences(_device->handle(), 1, &_fences[_frameIndex]));
	auto res = vkAcquireNextImageKHR(_device->handle(), swapchain.handle, UINT64_MAX, _presentComplete[_frameIndex], _fences[_frameIndex], &currentBuffer);

	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		_frameIndex += 1;
		_frameIndex %= FRAME_LAG;

		return;
	} else if (res == VK_SUBOPTIMAL_KHR) {

	} else {
		assert(res == VK_SUCCESS);
	}
	VK_CHECK_RESULT(vkWaitForFences(_device->handle(), 1, &_renderFences[_frameIndex], VK_TRUE, UINT64_MAX));
	VK_CHECK_RESULT(vkResetFences(_device->handle(), 1, &_renderFences[_frameIndex]));
	
	//BuildOffscreenCommandBuffer(_offscreenDraw[_frameIndex], world);

	VkPipelineStageFlags const pipeStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask = &pipeStageFlags;
	submitInfo.waitSemaphoreCount = 1;
	//submitInfo.pWaitSemaphores = &_presentComplete[_frameIndex];
	submitInfo.commandBufferCount = 1;
	//submitInfo.pCommandBuffers = &_offscreenDraw[_frameIndex];
	submitInfo.signalSemaphoreCount = 1;
	//submitInfo.pSignalSemaphores = &_offscreenRender[_frameIndex];
	
	//VK_CHECK_RESULT(vkQueueSubmit(_device->getGraphicsQueue(), 1, &submitInfo, VkFence()));

	BuildPresentCommandBuffer(_draw[_frameIndex], world);

	submitInfo.pWaitSemaphores = &_presentComplete[_frameIndex];
	submitInfo.pSignalSemaphores = &_completeRender[_frameIndex];
	submitInfo.pCommandBuffers = &_draw[_frameIndex];

	VK_CHECK_RESULT(vkQueueSubmit(_device->getGraphicsQueue(), 1, &submitInfo, _renderFences[_frameIndex]));

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &_completeRender[_frameIndex];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain.handle;
	presentInfo.pImageIndices = &currentBuffer;

	 res = vkQueuePresentKHR(_device->getPresentQueue(), &presentInfo);
	_frameIndex += 1;
	_frameIndex %= FRAME_LAG;
	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		return;
	} else if (res == VK_SUBOPTIMAL_KHR) {
		
	} else {
		assert(res == VK_SUCCESS);
	}
}

void Renderer::destroy()
{
	vkDeviceWaitIdle(_device->handle());
	_baseColour.destroy(_device);
	_normal.destroy(_device);
	_roughness.destroy(_device);
	_metallic.destroy(_device);
	_ao.destroy(_device);

	_environmentCube.destroy(_device);
	_lutBrdf.destroy(_device);
	_irradianceCube.destroy(_device);
	_prefilteredCube.destroy(_device);

	_lights.destroy(_device);
	_cameraSpace.destroy(_device);
	_positions.destroy(_device);
	_matPostion.destroy(_device);
	_cameraDescriptor.destroy(_device);
	_skyboxDescriptor.destroy(_device);
//	_deferredShaderDescriptor.destroy(_device);
	
	_presentShader.destroy(_device);
	_skyboxShader.destroy(_device);
//	delete _deferredShader;

	_monkey.destroy(_device);
	_plane.destroy(_device);
	_box.destroy(_device);

	PresentRenderTarget.destroy(_device);
	vkWaitForFences(_device->handle(), FRAME_LAG, _fences, VK_TRUE, UINT64_MAX);
	for (int i = 0; i < FRAME_LAG; i++) {
		vkDestroySemaphore(_device->handle(), _completeRender[i], nullptr);
		vkDestroySemaphore(_device->handle(), _presentComplete[i], nullptr);
		vkDestroySemaphore(_device->handle(), _offscreenRender[i], nullptr);
		vkDestroyFence(_device->handle(), _fences[i], nullptr);
		vkDestroyFence(_device->handle(), _renderFences[i], nullptr);
	}
	vkDestroySwapchainKHR(_device->handle(), swapchain.handle, nullptr);
	for (auto frame : swapchain.view) {
		vkDestroyImageView(_device->handle(), frame, nullptr);
	}
	vkDestroySurfaceKHR(instance, surface, nullptr);
	_device->deallocateAll();
	delete _device;
	vkDestroyInstance(instance, nullptr);
}

void Renderer::initInstance(std::string title, bool bwValidation, bool bwDebugReport) {
	std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };
	std::vector<const char*> layer;

#if defined(_WIN32)
	instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
    instanceExtensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
	
	instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	
	layer.push_back("VK_LAYER_LUNARG_standard_validation");

	dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)vulkanDebugCallback;
	dbgCreateInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = title.c_str();
	appInfo.pEngineName = title.c_str();
	appInfo.engineVersion = 0;
	appInfo.applicationVersion = 0;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 77);

	VkInstanceCreateInfo instInfo = {};
	instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instInfo.pApplicationInfo = &appInfo;
	instInfo.enabledLayerCount = layer.size();
	instInfo.ppEnabledLayerNames = layer.data();
	instInfo.enabledExtensionCount = instanceExtensions.size();
	instInfo.ppEnabledExtensionNames = instanceExtensions.data();
	instInfo.pNext = &dbgCreateInfo;

	VK_CHECK_RESULT(vkCreateInstance(&instInfo, NULL, &this->instance));
}

void Renderer::initDebug() {

	CreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
	CreateDebugReportCallbackEXT(this->instance, reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT*>(&dbgCreateInfo), NULL, &debugReport);
}

void Renderer::initDevice() {
	_device = new Device(instance, surface);
}

void Renderer::GetCapabilities() {
	uint32_t pSurfaceFormatCount;
	
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(_device->_gpu, this->surface, &pSurfaceFormatCount, NULL));
	std::vector<VkSurfaceFormatKHR> pSurfaceFormats(pSurfaceFormatCount);
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(_device->_gpu, this->surface, &pSurfaceFormatCount, pSurfaceFormats.data()));

	VkSurfaceFormatKHR format = {};

	for (int i = 0; i < pSurfaceFormatCount; i++) {
		if (pSurfaceFormatCount == 1 && pSurfaceFormats[i].format == VK_FORMAT_UNDEFINED) {
			format.format = VK_FORMAT_B8G8R8A8_UNORM;
			format.colorSpace = pSurfaceFormats[i].colorSpace;
		}
		else {
			format = pSurfaceFormats[i];
		}
	}

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device->_gpu, this->surface, &surfaceCapabilities);

	uint32_t desiredImageCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 && desiredImageCount > surfaceCapabilities.maxImageCount) {
		desiredImageCount = surfaceCapabilities.maxImageCount;
	}

	VkSurfaceTransformFlagBitsKHR preTransform;
	if (surfaceCapabilities.supportedTransforms &
		VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else {
		preTransform = surfaceCapabilities.currentTransform;
	}

	capabilities.capabilities = surfaceCapabilities;
	capabilities.preTransform = preTransform;
	capabilities.desiredImageCount = desiredImageCount;
	capabilities.format = format;
}

void Renderer::CreateSwapchain() {
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = capabilities.desiredImageCount;
	createInfo.imageColorSpace = capabilities.format.colorSpace;
	createInfo.imageFormat = capabilities.format.format;
	createInfo.imageExtent = capabilities.capabilities.maxImageExtent;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.preTransform = capabilities.preTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	createInfo.clipped = 1;
	createInfo.imageArrayLayers = 1;
	createInfo.queueFamilyIndexCount = 0;

	VK_CHECK_RESULT(vkCreateSwapchainKHR(_device->handle(), &createInfo, NULL, &this->swapchain.handle));

	uint32_t imageCount;
	vkGetSwapchainImagesKHR(_device->handle(), this->swapchain.handle, &imageCount, NULL);
	this->swapchain.images.resize(imageCount);
	this->swapchain.view.resize(imageCount);
	vkGetSwapchainImagesKHR(_device->handle(), this->swapchain.handle, &imageCount, this->swapchain.images.data());
	for (int i = 0; i < imageCount; i++) {
		this->swapchain.view[i] = VK_NULL_HANDLE;
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = capabilities.format.format;
		viewInfo.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		viewInfo.subresourceRange = {
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1
		};
		viewInfo.image = this->swapchain.images[i];
		VK_CHECK_RESULT(vkCreateImageView(_device->handle(), &viewInfo, NULL, &this->swapchain.view[i]));
	}
}

void Renderer::CreateFencesSemaphore() {
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32_t i = 0; i < FRAME_LAG; i++) {
		VK_CHECK_RESULT(vkCreateFence(_device->handle(), &fenceInfo, nullptr, &_fences[i]));
		VK_CHECK_RESULT(vkCreateFence(_device->handle(), &fenceInfo, nullptr, &_renderFences[i]));
		VK_CHECK_RESULT(vkCreateSemaphore(_device->handle(), &semaphoreCreateInfo, nullptr, &_completeRender[i]));
		std::string preText = "PresentCommandBuffer ";
		preText += std::to_string(i);
		_device->setSemaphoreName(_completeRender[i], preText.c_str());
		VK_CHECK_RESULT(vkCreateSemaphore(_device->handle(), &semaphoreCreateInfo, nullptr, &_presentComplete[i]));
		VK_CHECK_RESULT(vkCreateSemaphore(_device->handle(), &semaphoreCreateInfo, nullptr, &_offscreenRender[i]));
	}

	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.commandPool = _device->_commandPool;
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = swapchain.view.size() + FRAME_LAG;

	_draw.resize(swapchain.view.size() + FRAME_LAG);
	vkAllocateCommandBuffers(_device->handle(), &cmd, _draw.data() );
	for (int i = swapchain.view.size() + FRAME_LAG; i > swapchain.view.size(); i--) {
		_offscreenDraw[i - FRAME_LAG - 1] = _draw[i-1];
		_draw.pop_back();
	}
}

void Renderer::BuildPresentCommandBuffer(VkCommandBuffer commandBuffer, World* world){
	std::vector<Light> lights = world->getLights();
	for (int i = 0; i < lights.size(); i++) {
		_cameraMat.light[i] = lights[i];
	}
	Camera camera = world->getCamera();
	_cameraMat.view = glm::translate(glm::mat4(1.0f), camera.transform.loction);
	_cameraMat.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	_cameraMat.lightCount = lights.size();
	_cameraMat.viewPos = camera.transform.loction * -1.0f;
	_cameraMat.gamma = camera.gamma;
	_cameraMat.exposure = camera.exposure;

	_cameraSpace.update(_device, &_cameraMat);

	VkCommandBufferBeginInfo commandInfo = {};
	commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	
	auto const resolution = PresentRenderTarget.getResolution();

	VkClearValue clearValues[2] = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0u };

	VkRenderPassBeginInfo passInfo = {};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passInfo.renderPass = PresentRenderTarget.getRenderPass();
	passInfo.framebuffer = PresentRenderTarget.getFramebuffers()[currentBuffer];
	passInfo.renderArea.extent = resolution;
	passInfo.clearValueCount = 2;
	passInfo.pClearValues = clearValues;

	auto res = vkBeginCommandBuffer(commandBuffer, &commandInfo);
	assert(res == VK_SUCCESS);

	std::string text = "PresentCommandBuffer ";
	text += std::to_string(currentBuffer);

	_device->beginRegion(commandBuffer, text.c_str(), glm::vec4({ 1.0f, 0.0f, 0.0f, 1.0f }));

	vkCmdBeginRenderPass(commandBuffer, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _presentShader.GetPipeline());
	VkViewport viewport = {};
	viewport.height = resolution.height;
	viewport.width = resolution.width;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.extent = resolution;

	_cameraDescriptor.makeCurrent(commandBuffer, &_presentShader);

	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	_monkey.bind(commandBuffer);
	for (int i = 0; i < 25; i++) {
		_positions.makeCurrentAlign(commandBuffer, i, &_presentShader);
		_monkey.draw(commandBuffer);
	}
	/*
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _skyboxShader->GetPipeline());
	_skyboxDescriptor.makeCurrent(commandBuffer, _skyboxShader);
	_box.bind(commandBuffer);
	_box.draw(commandBuffer);
	*/
	vkCmdEndRenderPass(commandBuffer);
	_device->endRegion(commandBuffer);
	vkEndCommandBuffer(commandBuffer);
	assert(res == VK_SUCCESS);
}

void Renderer::BuildOffscreenCommandBuffer(VkCommandBuffer cmd, World* world)
{
	Camera camera = world->getCamera();
	CameraMat cameraMat = {};
	cameraMat.per = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f) * glm::translate(glm::mat4(), camera.transform.loction);
	_cameraSpace.update(_device, &cameraMat);
	VkCommandBufferBeginInfo commandInfo = {};
	commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	auto const resolution = DeferredRenderTarget.getResolution();

	VkClearValue clearValues[7] = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[2].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[3].color = { 0.0f };
	clearValues[4].color = { 0.0f };
	clearValues[5].color = { 0.0f };
	clearValues[6].depthStencil = { 1.0f, 0u };

	VkRenderPassBeginInfo passInfo = {};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passInfo.renderPass = DeferredRenderTarget.getRenderPass();
	passInfo.framebuffer = DeferredRenderTarget.getFramebuffers()[0];
	passInfo.renderArea.extent = resolution;
	passInfo.clearValueCount = 7;
	passInfo.pClearValues = clearValues;
	
	auto res = vkBeginCommandBuffer(cmd, &commandInfo);
	assert(res == VK_SUCCESS);

	_device->beginRegion(cmd, "OffscreenCommandBuffer", glm::vec4({ 1.0f, 0.0f, 0.0f, 1.0f }));

	vkCmdBeginRenderPass(cmd, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _deferredShader.GetPipeline());
	_cameraDescriptor.makeCurrent(cmd, &_deferredShader);
	
	VkViewport viewport = {};
	viewport.height = resolution.height;
	viewport.width = resolution.width;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.extent = resolution;

	vkCmdSetScissor(cmd, 0, 1, &scissor);
	_monkey.bind(cmd);
	_deferredShaderDescriptor.makeCurrent(cmd);
	for (int i = 0; i < 25; i++) {
		_positions.makeCurrentAlign(cmd, i, &_deferredShader);
		_monkey.draw(cmd);
	}
	
	vkCmdEndRenderPass(cmd);
	_device->endRegion(cmd);
	vkEndCommandBuffer(cmd);
	assert(res == VK_SUCCESS);
}

void Renderer::generateBRDFLUT() {
	const int32_t dim = 512;
	std::array<AttachmentInfo, 2> req{ {
		{ VK_FORMAT_R16G16_SFLOAT,			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1 },
		{ VK_FORMAT_D16_UNORM,			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1 }
	} };

	RenderTarget renderTarget = RenderTarget::Create(_device, { dim, dim }, req.data(), req.size());
	
	Shader _genBRDflut = Shader::Create(_device, &renderTarget, path("assets/shaders/passThrough.vert"), path("assets/shaders/genbrdflut.frag"), std::vector<ShaderLayout>());

	VkCommandBuffer cmd = VK_NULL_HANDLE;
	_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &cmd);
	VkCommandBufferBeginInfo cmdBufferBeginInfo{};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(cmd, &cmdBufferBeginInfo);

	VkClearValue clearValues[2];
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0u };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderTarget.getRenderPass();
	renderPassBeginInfo.renderArea.extent.width = dim;
	renderPassBeginInfo.renderArea.extent.height = dim;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;
	renderPassBeginInfo.framebuffer = renderTarget.getFramebuffers()[0];

	vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	VkViewport viewport = {};
	viewport.height = (float)dim;
	viewport.width = (float)dim;
	viewport.minDepth =  0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor = { 0, 0, dim, dim };
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _genBRDflut.GetPipeline());
	_plane.bind(cmd);
	_plane.draw(cmd);
	vkCmdEndRenderPass(cmd);
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;
//	_lutBrdf.setImageLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);
	_device->submitCommandBuffer(cmd, true);
	_lutBrdf = renderTarget.takeAttachment(_device, 0);
	_genBRDflut.destroy(_device);
}

void Renderer::generateIrradianceCube()
{
	const int32_t dim = 512;
	const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

	_irradianceCube = Texture::CreateFromDim(_device, dim, 6, numMips, VK_FORMAT_R32G32B32A32_SFLOAT, true);

	std::array<AttachmentInfo, 2> req{ {
		{ VK_FORMAT_R32G32B32A32_SFLOAT,	VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1 },
		{ VK_FORMAT_D16_UNORM,				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1 }
		}
	};

	RenderTarget renderTarget = RenderTarget::Create(_device, { dim, dim }, req.data(), req.size());

	struct PushBlock {
		glm::mat4 mvp;
		float deltaPhi = (2.0f * float(M_PI)) / 180.0f;
		float deltaTheta = (0.5f * float(M_PI)) / 64.0f;
	} pushBlock;

	PushBlock block = {};

	std::vector<VkPushConstantRange> layoutConts;
	VkPushConstantRange push = {};
	push.offset = 0;
	push.size = sizeof(PushBlock);
	push.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutConts.push_back(push);

	UniformBuffer blockBinding = UniformBuffer::CreateUniformBuffer(_device, block);
	vector<UniformBinding> bindings = {
		UniformBinding(&_environmentCube, 0)
	};

	std::vector<ShaderLayout> layout;
	layout.push_back(ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 0));

	Shader _genBRDflut = Shader::Create(_device, &renderTarget, path("assets/shaders/filtercube.vert"), path("assets/shaders/irradiancecube.frag"), layout, layoutConts);

	ShaderDescriptor shaderDescriptor = ShaderDescriptor(_device, &_genBRDflut, bindings);

	std::vector<glm::mat4> matrices = {
		// POSITIVE_X
		glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_X
		glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// POSITIVE_Y
		glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_Y
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// POSITIVE_Z
		glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_Z
		glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
	};

	VkCommandBuffer cmd = VK_NULL_HANDLE;
	_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &cmd);
	VkCommandBufferBeginInfo cmdBufferBeginInfo{};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(cmd, &cmdBufferBeginInfo);

	VkClearValue clearValues[2];
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0u };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderTarget.getRenderPass();
	renderPassBeginInfo.renderArea.extent.width = dim;
	renderPassBeginInfo.renderArea.extent.height = dim;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;
	renderPassBeginInfo.framebuffer = renderTarget.getFramebuffers()[0];

	VkRect2D scissor = { 0, 0, dim, dim };
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = numMips;
	subresourceRange.layerCount = 6;

	_irradianceCube.setImageLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

	for (uint32_t m = 0; m < numMips; m++) {
		VkViewport viewport = {};
		viewport.height = static_cast<float>(dim * std::pow(0.5f, m));
		viewport.width = static_cast<float>(dim * std::pow(0.5f, m));
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		for (uint32_t f = 0; f < 6; f++) {
			pushBlock.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
			blockBinding.update(_device, &pushBlock);
			vkCmdSetViewport(cmd, 0, 1, &viewport);

			vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdPushConstants(cmd, _genBRDflut.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlock), &pushBlock);
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _genBRDflut.GetPipeline());
			shaderDescriptor.makeCurrent(cmd);
			_box.bind(cmd);

			_box.draw(cmd);

			vkCmdEndRenderPass(cmd);

			VkImageSubresourceRange plah = {};
			plah.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			plah.baseMipLevel = 0;
			plah.levelCount = 1;
			plah.layerCount = 1;

			renderTarget.getAttachments()[0].setImageLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, plah);

			VkImageCopy copyRegion = {};

			copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.srcSubresource.baseArrayLayer = 0;
			copyRegion.srcSubresource.mipLevel = 0;
			copyRegion.srcSubresource.layerCount = 1;
			copyRegion.srcOffset = { 0, 0, 0 };

			copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.dstSubresource.baseArrayLayer = f;
			copyRegion.dstSubresource.mipLevel = m;
			copyRegion.dstSubresource.layerCount = 1;
			copyRegion.dstOffset = { 0, 0, 0 };

			copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
			copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
			copyRegion.extent.depth = 1;

			_irradianceCube.copyImage(cmd, &renderTarget.getAttachments()[0], 1, &copyRegion);
			renderTarget.getAttachments()[0].setImageLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, plah);
		}
	}

	_irradianceCube.setImageLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);

	_device->submitCommandBuffer(cmd, true);

	renderTarget.destroy(_device);
	blockBinding.destroy(_device);
	shaderDescriptor.destroy(_device);
	_genBRDflut.destroy(_device);
}

void Renderer::generatePrefilteredCube()
{
	auto tStart = std::chrono::high_resolution_clock::now();

	const int32_t dim = 512;
	const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

	_prefilteredCube = Texture::CreateFromDim(_device, dim, 6, numMips, VK_FORMAT_R16G16B16A16_SFLOAT, true);

	std::array<AttachmentInfo, 2> req{ {
		{ VK_FORMAT_R16G16B16A16_SFLOAT,	VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1 },
		{ VK_FORMAT_D16_UNORM,				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,							VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1 }
		}
	};

	std::vector<ShaderLayout> layout;
	layout.push_back(ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 0));

	struct PushBlock {
		glm::mat4 mvp;
		float roughness;
		uint32_t numSamples = 32u;
	} pushBlock;

	std::vector<VkPushConstantRange> layoutConts;
	VkPushConstantRange push = {};
	push.offset = 0;
	push.size = sizeof(PushBlock);
	push.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutConts.push_back(push);
	RenderTarget renderTarget = RenderTarget::Create(_device, { dim, dim }, req.data(), req.size());
	Shader _genBRDflut = Shader::Create(_device, &renderTarget, path("assets/shaders/filtercube.vert"), path("assets/shaders/prefilterenvmap.frag"), layout, layoutConts);


	PushBlock block = {};

	UniformBuffer blockBinding = UniformBuffer::CreateUniformBuffer(_device, block);
	vector<UniformBinding> bindings = {
		UniformBinding(&_environmentCube, 0)
	};

	ShaderDescriptor shaderDescriptor = ShaderDescriptor(_device, &_genBRDflut, bindings);

	std::vector<glm::mat4> matrices = {
		// POSITIVE_X
		glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_X
		glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// POSITIVE_Y
		glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_Y
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// POSITIVE_Z
		glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_Z
		glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
	};

	VkCommandBuffer cmd = VK_NULL_HANDLE;
	_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &cmd);
	VkCommandBufferBeginInfo cmdBufferBeginInfo{};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(cmd, &cmdBufferBeginInfo);

	VkClearValue clearValues[2];
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0u };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderTarget.getRenderPass();
	renderPassBeginInfo.renderArea.extent.width = dim;
	renderPassBeginInfo.renderArea.extent.height = dim;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;
	renderPassBeginInfo.framebuffer = renderTarget.getFramebuffers()[0];
	
	VkRect2D scissor = { 0, 0, dim, dim };
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = numMips;
	subresourceRange.layerCount = 6;

	_prefilteredCube.setImageLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

	for (uint32_t m = 0; m < numMips; m++) {
		pushBlock.roughness = (float)m / (float)(numMips - 1);
		VkViewport viewport = {};
		viewport.height = static_cast<float>(dim * std::pow(0.5f, m));
		viewport.width = static_cast<float>(dim * std::pow(0.5f, m));
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		for (uint32_t f = 0; f < 6; f++) {
			pushBlock.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
			//blockBinding->update(_device, &pushBlock);
			vkCmdSetViewport(cmd, 0, 1, &viewport);

			vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdPushConstants(cmd, _genBRDflut.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlock), &pushBlock);
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _genBRDflut.GetPipeline());
			shaderDescriptor.makeCurrent(cmd);
			_box.bind(cmd);

			_box.draw(cmd);

			vkCmdEndRenderPass(cmd);

			VkImageSubresourceRange plah = {};
			plah.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			plah.baseMipLevel = 0;
			plah.levelCount = 1;
			plah.layerCount = 1;

			renderTarget.getAttachments()[0].setImageLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, plah);

			VkImageCopy copyRegion = {};

			copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.srcSubresource.baseArrayLayer = 0;
			copyRegion.srcSubresource.mipLevel = 0;
			copyRegion.srcSubresource.layerCount = 1;
			copyRegion.srcOffset = { 0, 0, 0 };

			copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.dstSubresource.baseArrayLayer = f;
			copyRegion.dstSubresource.mipLevel = m;
			copyRegion.dstSubresource.layerCount = 1;
			copyRegion.dstOffset = { 0, 0, 0 };

			copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
			copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
			copyRegion.extent.depth = 1;

			_prefilteredCube.copyImage(cmd, &renderTarget.getAttachments()[0], 1, &copyRegion);
			renderTarget.getAttachments()[0].setImageLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, plah);

		}
	}

	_prefilteredCube.setImageLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);

	_device->submitCommandBuffer(cmd, true);
	auto tEnd = std::chrono::high_resolution_clock::now();
	auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
	std::cout << "Generating pre-filtered enivornment cube with " << numMips << " mip levels took " << tDiff << " ms" << std::endl;

	renderTarget.destroy(_device);
	blockBinding.destroy(_device);
	shaderDescriptor.destroy(_device);
	_genBRDflut.destroy(_device);
}
