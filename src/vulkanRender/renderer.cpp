#include "renderer.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../windowHandler.h"
#include "util.h"
#include <vulkan/vulkan.h>

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

Renderer::Renderer(std::string title, WindowHandle* window) {
	initInstance(title);
	printf("initInstance\n");
	surface = window->createSurface(instance);
	printf("createSurface\n");
	initDevice();
	printf("initDevice\n");
	GetCapabilities();
	CreateSwapchain();
	AttachmentInfo defferedAttachmentInfo[] = {
		{ VK_FORMAT_R16G16B16A16_SFLOAT,	VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1 },
		{ VK_FORMAT_R16G16B16A16_SFLOAT,	VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1 },
		{ VK_FORMAT_R8G8B8A8_UNORM,		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1 },
		{ VK_FORMAT_D16_UNORM,			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1 }
	};

	AttachmentInfo PresentAttachmentInfo[] = {
		{ capabilities.format.format,		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1 },
		{ VK_FORMAT_D16_UNORM,			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,	VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1 } };

	VkExtent2D resolution = capabilities.capabilities.maxImageExtent;

	PresentRenderTarget = RenderTarget::Create(_device, resolution, PresentAttachmentInfo, 2, &swapchain.view);
	DeferredRenderTarget = RenderTarget::Create(_device, resolution, defferedAttachmentInfo, 4);
	_texture = Texture::Create(_device, path("assets/textures/MarbleGreen_COLOR.tga"));
	_plane = Mesh::Create(_device, path("assets/Mesh/plane.dae"));
	_monkey = Mesh::Create(_device, path("assets/Mesh/monkey.dae"));
	printf("load mesh complete\n");
	//_monkey->setBufferName(_device, "monkey");
	std::vector<ShaderLayout> deferredLayout(2);
	deferredLayout[0] = ShaderLayout(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 0);
	deferredLayout[1] = ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);
	
	std::vector<ShaderLayout> presentLayout(3);
	presentLayout[0] = ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 0);
	presentLayout[1] = ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 0);
	presentLayout[2] = ShaderLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2, 0);

	printf("Create Shader\n");
	_presentShader	= Shader::Create(_device, PresentRenderTarget, path("assets/shaders/present.vert"), path("assets/shaders/present.frag"), presentLayout);
	_deferredShader = Shader::Create(_device, DeferredRenderTarget, path("assets/shaders/deferred.vert"), path("assets/shaders/deferred.frag"), deferredLayout);
	std::vector<UniformBinding> _presentUniforms = {
		{ DeferredRenderTarget->getAttachments()[0], 0, 0},
		{ DeferredRenderTarget->getAttachments()[1], 1, 0},
		{ DeferredRenderTarget->getAttachments()[2], 2, 0},
	};
	glm::mat4 matPerspec = glm::perspective(glm::radians(90.0f), (float)resolution.width / (float)resolution.height, 0.1f, 100.0f);
	glm::mat4 matTran = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -2.0f));
	glm::mat4 matRot = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	glm::mat4 myMatrix = matPerspec * matTran;
	_cameraSpace = UniformBuffer::CreateUniformBuffer(_device, myMatrix);
	std::vector<UniformBinding> _deferredUniforms = { {_cameraSpace,  0, 0}, { _texture, 1, 1} };
	printf("Create _presentMaterial\n");
	_presentMaterial = Material::CreateMaterialWithShader(_device, _presentShader, _presentUniforms);
	printf("Create _presentMaterial\n");
	_deferredMaterial = Material::CreateMaterialWithShader(_device, _deferredShader, _deferredUniforms);
	printf("begin CreateFencesSemaphores\n");
	CreateFencesSemaphore();

	prepared = true;
}

Renderer::~Renderer() {
	vkDeviceWaitIdle(_device->handle());
	_texture->destroy(_device);
	delete _texture;
	delete _cameraSpace;
	delete _presentShader;
	delete _deferredShader;
	_monkey->destroy(_device);
	delete _monkey;
	_plane->destroy(_device);
	delete _plane;
	delete PresentRenderTarget;
	delete DeferredRenderTarget;
	delete _presentMaterial;
	delete _deferredMaterial;
	vkWaitForFences(_device->handle(), FRAME_LAG, _fences, VK_TRUE, UINT64_MAX);
	for (int i = 0; i < FRAME_LAG; i++) {
		vkDestroySemaphore(_device->handle(), _completeRender[i], nullptr);
		vkDestroySemaphore(_device->handle(), _presentComplete[i], nullptr);
		vkDestroyFence(_device->handle(), _fences[i], nullptr);
	}
	vkDestroySemaphore(_device->handle(), _offscreenRender, nullptr);
	vkDestroySwapchainKHR(_device->handle(), swapchain.handle, nullptr);
	for (auto frame : swapchain.view) {
		vkDestroyImageView(_device->handle(), frame, nullptr);
	}
	vkDestroySurfaceKHR(instance, surface, nullptr);
	_device->deallocateAll();
	delete _device;
	vkDestroyInstance(instance, nullptr);
}

void Renderer::Run() {
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

	VkPipelineStageFlags const pipeStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask = &pipeStageFlags;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &_presentComplete[_frameIndex];
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_offscreenDraw;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &_offscreenRender;
	
	VK_CHECK_RESULT(vkQueueSubmit(_device->getGraphicsQueue(), 1, &submitInfo, VkFence()));
	submitInfo.pWaitSemaphores = &_offscreenRender;
	submitInfo.pSignalSemaphores = &_completeRender[_frameIndex];
	submitInfo.pCommandBuffers = &_draw[_frameIndex];

	VK_CHECK_RESULT(vkQueueSubmit(_device->getGraphicsQueue(), 1, &submitInfo, VkFence()));

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

void Renderer::initInstance(std::string title) {
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
	appInfo.apiVersion = VK_API_VERSION_1_0;

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
		VK_CHECK_RESULT(vkCreateSemaphore(_device->handle(), &semaphoreCreateInfo, nullptr, &_completeRender[i]));
		std::string preText = "PresentCommandBuffer ";
		preText += std::to_string(i);
		_device->setSemaphoreName(_completeRender[i], preText.c_str());
		VK_CHECK_RESULT(vkCreateSemaphore(_device->handle(), &semaphoreCreateInfo, nullptr, &_presentComplete[i]));
	}
	VK_CHECK_RESULT(vkCreateSemaphore(_device->handle(), &semaphoreCreateInfo, nullptr, &_offscreenRender));

	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.commandPool = _device->_commandPool;
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = swapchain.view.size() + 1;

	_draw.resize(swapchain.view.size() + 1);
	vkAllocateCommandBuffers(_device->handle(), &cmd, _draw.data() );
	_offscreenDraw = _draw[swapchain.view.size()];
	_draw.pop_back();
	for (auto& cmd : _draw) {
		BuildPresentCommandBuffer(cmd);
	}
	BuildOffscreenCommandBuffer();
}

void Renderer::BuildPresentCommandBuffer(VkCommandBuffer commandBuffer){
	VkCommandBufferBeginInfo commandInfo = {};
	commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	
	auto const resolution = PresentRenderTarget->getResolution();

	VkClearValue clearValues[2] = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0u };

	VkRenderPassBeginInfo passInfo = {};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passInfo.renderPass = PresentRenderTarget->getRenderPass();
	passInfo.framebuffer = PresentRenderTarget->getFramebuffers()[currentBuffer];
	passInfo.renderArea.extent = resolution;
	passInfo.clearValueCount = 2;
	passInfo.pClearValues = clearValues;

	auto res = vkBeginCommandBuffer(commandBuffer, &commandInfo);
	assert(res == VK_SUCCESS);

	std::string text = "PresentCommandBuffer ";
	text += std::to_string(currentBuffer);

	_device->beginRegion(commandBuffer, text.c_str(), glm::vec4({ 1.0f, 0.0f, 0.0f, 1.0f }));

	vkCmdBeginRenderPass(commandBuffer, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _presentShader->GetPipeline());
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _presentShader->GetPipelineLayout(), 0, _presentMaterial->getDescriptorSets()->size(), _presentMaterial->getDescriptorSets()->data(), 0, nullptr);

	VkViewport viewport = {};
	viewport.height = resolution.height;
	viewport.width = resolution.width;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.extent = resolution;


	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	_plane->draw(commandBuffer);
	vkCmdEndRenderPass(commandBuffer);
	_device->endRegion(commandBuffer);
	vkEndCommandBuffer(commandBuffer);
	assert(res == VK_SUCCESS);
	currentBuffer = currentBuffer + 1;
}

void Renderer::BuildOffscreenCommandBuffer()
{
	VkCommandBufferBeginInfo commandInfo = {};
	commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	auto const resolution = DeferredRenderTarget->getResolution();

	VkClearValue clearValues[4] = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[2].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[3].depthStencil = { 1.0f, 0u };

	VkRenderPassBeginInfo passInfo = {};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passInfo.renderPass = DeferredRenderTarget->getRenderPass();
	passInfo.framebuffer = DeferredRenderTarget->getFramebuffers()[0];
	passInfo.renderArea.extent = resolution;
	passInfo.clearValueCount = 4;
	passInfo.pClearValues = clearValues;
	
	auto res = vkBeginCommandBuffer(_offscreenDraw, &commandInfo);
	assert(res == VK_SUCCESS);

	_device->beginRegion(_offscreenDraw, "OffscreenCommandBuffer", glm::vec4({ 1.0f, 0.0f, 0.0f, 1.0f }));

	vkCmdBeginRenderPass(_offscreenDraw, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(_offscreenDraw, VK_PIPELINE_BIND_POINT_GRAPHICS, _deferredShader->GetPipeline());
	vkCmdBindDescriptorSets(_offscreenDraw, VK_PIPELINE_BIND_POINT_GRAPHICS, _deferredShader->GetPipelineLayout(), 0, _deferredMaterial->getDescriptorSets()->size(), _deferredMaterial->getDescriptorSets()->data(), 0, nullptr);

	VkViewport viewport = {};
	viewport.height = resolution.height;
	viewport.width = resolution.width;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(_offscreenDraw, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.extent = resolution;

	vkCmdSetScissor(_offscreenDraw, 0, 1, &scissor);
	_monkey->draw(_offscreenDraw);
	vkCmdEndRenderPass(_offscreenDraw);
	_device->endRegion(_offscreenDraw);
	vkEndCommandBuffer(_offscreenDraw);
	assert(res == VK_SUCCESS);
}
