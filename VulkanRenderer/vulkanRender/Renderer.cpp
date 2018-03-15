#include "Renderer.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <glm\glm.hpp>
#include "../Window.h"
#include "util.h"

#include "resources\UniformBuffer.h"

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

Renderer::Renderer(std::string title, Window* window) {
	initInstance(title);
	surface = window->createSurface(instance);
	initDevice();
	GetCapabilities();
	CreateSwapchain();
	/*
	std::vector<AttachmentInfo> defferedAttachmentInfo = {
		{ vk::Format::eR16G16B16A16Sfloat,	vk::ImageUsageFlagBits::eColorAttachment, vk::ImageLayout::eColorAttachmentOptimal, 1 },
		{ vk::Format::eR16G16B16A16Sfloat,	vk::ImageUsageFlagBits::eColorAttachment, vk::ImageLayout::eColorAttachmentOptimal, 1 },
		{ vk::Format::eR8G8B8A8Unorm,		vk::ImageUsageFlagBits::eColorAttachment, vk::ImageLayout::eColorAttachmentOptimal, 1 },
		{ vk::Format::eD16Unorm,			vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal, 1 }
	};

	std::vector<Texture*> attachments(defferedAttachmentInfo.size());
	for (int i = 0; i < defferedAttachmentInfo.size(); i++) {
		attachments[i] = Texture::Create(
			_device,
			capabilities.capabilities.maxImageExtent,
			defferedAttachmentInfo[i].format,
			defferedAttachmentInfo[i].usage,
			defferedAttachmentInfo[i].imageLayout,
			nullptr);
	}
	
	_device->allocate(*(std::vector<Resource*> *)&attachments, vk::MemoryPropertyFlagBits::eDeviceLocal);
	renderTarget = RenderTarget::CreateFromTextures(_device, attachments);
	*/

	std::vector<AttachmentInfo> attachmentInfo = {
		{ capabilities.format.format,		vk::ImageUsageFlagBits::eColorAttachment,			vk::ImageLayout::ePresentSrcKHR, 1 },
		{ vk::Format::eD16Unorm,			vk::ImageUsageFlagBits::eDepthStencilAttachment,	vk::ImageLayout::eDepthStencilAttachmentOptimal, 1 } };

	std::vector<Texture*> attachments(attachmentInfo.size());
	for (int i = 0; i < attachments.size(); i++) {
		attachments[i] = Texture::Create(_device, capabilities.capabilities.maxImageExtent, attachmentInfo[i].format, attachmentInfo[i].usage, attachmentInfo[i].imageLayout, nullptr);
	}
	//_device->allocate(*(std::vector<Resource*> *)&attachments, vk::MemoryPropertyFlagBits::eDeviceLocal);
	renderTarget = RenderTarget::Create(_device, capabilities.capabilities.maxImageExtent, attachmentInfo, &swapchain.view);
	
	_mesh = Mesh::Create(_device, path("assets/Mesh/monkey.dae"));
	std::vector<ShaderLayout> shaderLayout(1);
	_texture = Texture::Create(_device, path("assets/textures/MarbleGreen_COLOR.tga"));
	shaderLayout[0] = ShaderLayout(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 0, 0);
	_presentShader = Shader::Create(_device, renderTarget, path("assets/shaders/deferred.vert"), path("assets/shaders/deferred.frag"), shaderLayout);
	_unfirom = UniformBuffer::CreateUniformBuffer<glm::vec3>(_device, glm::vec3(0.0f, 1.0f, 0.0f));
	std::vector<UniformBinding> uniforms = { { _texture, 0} };
	_material = Material::CreateMaterialWithShader(_device, _presentShader, uniforms);
	CreateFencesSemaphore();

	prepared = true;
}

Renderer::~Renderer() {
	_device->waitIdle();
	_texture->destroy(_device);
	delete _texture;
	delete _unfirom;
	delete _presentShader;
	_mesh->destroy(_device);
	delete _mesh;
	delete renderTarget;
	delete _material;
	_device->waitForFences(FRAME_LAG, _fences, VK_TRUE, UINT64_MAX);
	for (int i = 0; i < FRAME_LAG; i++) {
		_device->destroySemaphore(_completeRender[i]);
		_device->destroySemaphore(_presentComplete[i]);
		_device->destroyFence(_fences[i]);
	}
	_device->destroySwapchainKHR(swapchain.handle);
	for (auto frame : swapchain.view) {
		_device->destroyImageView(frame);
	}
	instance.destroySurfaceKHR(surface);
	_device->deallocateAll();
	_device->destroy();
	instance.destroy();
}

void Renderer::Run() {
	if (!prepared) {
		return;
	}
	VK_CHECK_RESULT(_device->waitForFences(1, &_fences[_frameIndex], VK_TRUE, UINT64_MAX));
	VK_CHECK_RESULT(_device->resetFences(1, &_fences[_frameIndex]));
	auto res = _device->acquireNextImageKHR(swapchain.handle, UINT64_MAX, _presentComplete[_frameIndex], _fences[_frameIndex], &currentBuffer);

	if (res == vk::Result::eErrorOutOfDateKHR) {
		_frameIndex += 1;
		_frameIndex %= FRAME_LAG;

		return;
	} else if (res == vk::Result::eSuboptimalKHR) {

	} else {
		assert(res == vk::Result::eSuccess);
	}

	vk::PipelineStageFlags const pipeStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	auto const submitInfo = vk::SubmitInfo()
		.setPWaitDstStageMask(&pipeStageFlags)
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&_presentComplete[_frameIndex])
		.setCommandBufferCount(1)
		.setPCommandBuffers(&_draw[currentBuffer])
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&_completeRender[_frameIndex]);

	VK_CHECK_RESULT(_device->getGraphicsQueue().submit(1, &submitInfo, vk::Fence()));

	auto const presentInfo = vk::PresentInfoKHR()
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&_completeRender[_frameIndex])
		.setSwapchainCount(1)
		.setPSwapchains(&swapchain.handle)
		.setPImageIndices(&currentBuffer);

	 res = _device->getPresentQueue().presentKHR(&presentInfo);
	_frameIndex += 1;
	_frameIndex %= FRAME_LAG;
	if (res == vk::Result::eErrorOutOfDateKHR) {
		return;
	} else if (res == vk::Result::eSuboptimalKHR) {
		
	} else {
		assert(res == vk::Result::eSuccess);
	}
}

void Renderer::initInstance(std::string title) {
	std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };
	std::vector<const char*> layer;

#if defined(_WIN32)
	instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

	instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	layer.push_back("VK_LAYER_LUNARG_standard_validation");

	dbgCreateInfo
		.setPfnCallback((PFN_vkDebugReportCallbackEXT)vulkanDebugCallback)
		.setFlags(vk::DebugReportFlagBitsEXT::eWarning);

	auto const appInfo = vk::ApplicationInfo()
		.setPApplicationName(title.c_str())
		.setPEngineName(title.c_str())
		.setEngineVersion(0)
		.setApplicationVersion(0)
		.setApiVersion(VK_MAKE_VERSION(1, 0, 65));

	auto const inst_info = vk::InstanceCreateInfo()
		.setPApplicationInfo(&appInfo)
		.setEnabledLayerCount(layer.size())
		.setPpEnabledLayerNames(layer.data())
		.setEnabledExtensionCount((uint32_t)instanceExtensions.size())
		.setPpEnabledExtensionNames(instanceExtensions.data())
		.setPNext(&dbgCreateInfo);

	VK_CHECK_RESULT(vk::createInstance(&inst_info, NULL, &this->instance));
}

void Renderer::initDebug() {
	CreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(instance.getProcAddr("vkCreateDebugReportCallbackEXT"));
	CreateDebugReportCallbackEXT(this->instance, reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT*>(&dbgCreateInfo), NULL, &debugReport);
}

void Renderer::initDevice() {
	_device = new EnDevice(instance, surface);
}

void Renderer::GetCapabilities() {
	uint32_t pSurfaceFormatCount;
	VK_CHECK_RESULT(_device->_gpu.getSurfaceFormatsKHR(this->surface, &pSurfaceFormatCount, NULL));
	std::vector<vk::SurfaceFormatKHR> pSurfaceFormats(pSurfaceFormatCount);
	VK_CHECK_RESULT(_device->_gpu.getSurfaceFormatsKHR(this->surface, &pSurfaceFormatCount, pSurfaceFormats.data()));

	vk::SurfaceFormatKHR format = {};

	for (int i = 0; i < pSurfaceFormatCount; i++) {
		if (pSurfaceFormatCount == 1 && pSurfaceFormats[i].format == vk::Format::eUndefined) {
			format.format = vk::Format::eB8G8R8A8Unorm;
			format.colorSpace = pSurfaceFormats[i].colorSpace;
		}
		else {
			format = pSurfaceFormats[i];
		}
	}

	vk::SurfaceCapabilitiesKHR surfaceCapabilities;
	_device->_gpu.getSurfaceCapabilitiesKHR(this->surface, &surfaceCapabilities);

	uint32_t desiredImageCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 && desiredImageCount > surfaceCapabilities.maxImageCount) {
		desiredImageCount = surfaceCapabilities.maxImageCount;
	}

	vk::SurfaceTransformFlagBitsKHR preTransform;
	if (surfaceCapabilities.supportedTransforms &
		vk::SurfaceTransformFlagBitsKHR::eIdentity) {
		preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
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
	auto const createInfo = vk::SwapchainCreateInfoKHR()
		.setSurface(surface)
		.setMinImageCount(capabilities.desiredImageCount)
		.setImageColorSpace(capabilities.format.colorSpace)
		.setImageFormat(capabilities.format.format)
		.setImageExtent(capabilities.capabilities.maxImageExtent)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setPreTransform(capabilities.preTransform)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(vk::PresentModeKHR::eFifo)
		.setClipped(1)
		.setImageArrayLayers(1)
		.setQueueFamilyIndexCount(0);

	VK_CHECK_RESULT(_device->createSwapchainKHR(&createInfo, NULL, &this->swapchain.handle));

	uint32_t imageCount;
	_device->getSwapchainImagesKHR(this->swapchain.handle, &imageCount, NULL);
	this->swapchain.images.resize(imageCount);
	this->swapchain.view.resize(imageCount);
	_device->getSwapchainImagesKHR(this->swapchain.handle, &imageCount, this->swapchain.images.data());
	for (int i = 0; i < imageCount; i++) {
		auto const viewInfo = vk::ImageViewCreateInfo()
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(capabilities.format.format)
			.setComponents(vk::ComponentMapping()
				.setR(vk::ComponentSwizzle::eR)
				.setG(vk::ComponentSwizzle::eG)
				.setB(vk::ComponentSwizzle::eB)
				.setA(vk::ComponentSwizzle::eA)
			)
			.setSubresourceRange(vk::ImageSubresourceRange(
				vk::ImageAspectFlagBits::eColor,
				0,
				1,
				0,
				1
			))
			.setImage(this->swapchain.images[i]);
		VK_CHECK_RESULT(_device->createImageView(&viewInfo, NULL, &this->swapchain.view[i]));
	}
}

void Renderer::CreateFencesSemaphore() {
	auto const semaphoreCreateInfo = vk::SemaphoreCreateInfo();
	auto const fence_ci = vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled);
	for (uint32_t i = 0; i < FRAME_LAG; i++) {
		VK_CHECK_RESULT(_device->createFence(&fence_ci, nullptr, &_fences[i]));
		VK_CHECK_RESULT(_device->createSemaphore(&semaphoreCreateInfo, nullptr, &_completeRender[i]));
		VK_CHECK_RESULT(_device->createSemaphore(&semaphoreCreateInfo, nullptr, &_presentComplete[i]));
	}

	auto const cmd = vk::CommandBufferAllocateInfo()
		.setCommandPool(_device->_commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(swapchain.view.size());

	_draw.resize(swapchain.view.size());
	_device->allocateCommandBuffers(&cmd, _draw.data() );

	for (auto& cmd : _draw) {
		BuildPresentCommandBuffer(cmd);
	}
}

void Renderer::BuildPresentCommandBuffer(vk::CommandBuffer commandBuffer){
	auto const commandInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	vk::ClearValue const clearValues[2] = { vk::ClearColorValue(std::array<float, 4>({ { 0.0f, 0.0f, 0.1f, 1.0f } })),
											vk::ClearDepthStencilValue(1.0f, 0u) };
	auto const passInfo = vk::RenderPassBeginInfo()
		.setRenderPass(renderTarget->getRenderPass())
		.setFramebuffer(renderTarget->getFramebuffers()[currentBuffer])
		.setRenderArea(vk::Rect2D(vk::Offset2D(), renderTarget->getResolution()))
		.setClearValueCount(2)
		.setPClearValues(clearValues);

	auto res = commandBuffer.begin(&commandInfo);
	assert(res == vk::Result::eSuccess);

	commandBuffer.beginRenderPass(&passInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _presentShader->GetPipeline());
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _presentShader->GetPipelineLayout(), 0, 1, &_material->getDescriptorSet(), 0, nullptr);

	auto const resolution = renderTarget->getResolution();

	auto const viewport =
		vk::Viewport().setWidth(resolution.width).setHeight(resolution.height).setMinDepth(0.0f).setMaxDepth(1.0f);
	commandBuffer.setViewport(0, 1, &viewport);

	vk::Rect2D const scissor(vk::Offset2D(0, 0), resolution);
	commandBuffer.setScissor(0, 1, &scissor);
	_mesh->draw(commandBuffer);
	commandBuffer.endRenderPass();
	commandBuffer.end();
	assert(res == vk::Result::eSuccess);
	currentBuffer = currentBuffer + 1;
}