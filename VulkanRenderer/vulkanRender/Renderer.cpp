#include "Renderer.h"
#include <vector>
#include <iostream>
#include <sstream>
#include "../Window.h"
#include "util.h"

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
	std::vector<AttachmentInfo> colourInfo = { { capabilities.format.format, vk::ImageUsageFlagBits::eColorAttachment, vk::ImageLayout::ePresentSrcKHR, 1 }};
	AttachmentInfo depthInfo = { vk::Format::eD16Unorm, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal, 1 };
	renderTarget = new RenderTarget(_device, capabilities.capabilities.maxImageExtent, colourInfo, depthInfo, &swapchain.view);
	resourceManger = new ResourceManger();
	_mesh = Mesh::Create(_device, path("assets/Mesh/monkey.dae"));
	_shader = Shader::Create(_device, renderTarget, path("assets/shaders/Test.vert"), path("assets/shaders/Test.frag"), std::vector<ShaderLayout>());
	CreateFencesSemaphore();

	prepared = true;
}

Renderer::~Renderer() {
	delete _shader;
	delete _mesh;
	delete resourceManger;
	delete renderTarget;
	_device->destroyCommandPool(_commandPool);
	_device->destroySwapchainKHR(swapchain.handle);
	instance.destroySurfaceKHR(surface);
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

	VK_CHECK_RESULT(graphicsQueue.submit(1, &submitInfo, vk::Fence()));

	auto const presentInfo = vk::PresentInfoKHR()
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&_completeRender[_frameIndex])
		.setSwapchainCount(1)
		.setPSwapchains(&swapchain.handle)
		.setPImageIndices(&currentBuffer);

	 res = presentQueue.presentKHR(&presentInfo);
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

#if defined(_DEBUG)
	instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	layer.push_back("VK_LAYER_LUNARG_standard_validation");
#endif
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
	uint32_t gpu_count;
	instance.enumeratePhysicalDevices(&gpu_count, NULL);
	std::vector<vk::PhysicalDevice> physicalDevices(gpu_count);

	if (gpu_count > 0) {
		instance.enumeratePhysicalDevices(&gpu_count, physicalDevices.data());
	}
	else {
		assert(0 && "no valid gpu");
	}

	this->gpu = physicalDevices[0];
	gpu.getQueueFamilyProperties(&queueFamilyCount, NULL);
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	gpu.getQueueFamilyProperties(&queueFamilyCount, queueFamilyProperties.data());

	std::vector<vk::Bool32> supportPresents(queueFamilyCount);
	graphicsQueueFamilyIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		gpu.getSurfaceSupportKHR(i, this->surface, &supportPresents[i]);
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics && supportPresents[i] == VK_TRUE) {
			graphicsQueueFamilyIndex = i;
		}
	}

	if (graphicsQueueFamilyIndex == UINT32_MAX) {
		assert(0 && "Vulkan no valid graphics queue family index.");
	}

	float queue_priorities[1] = { 0.0 };
	auto const queueInfo = vk::DeviceQueueCreateInfo()
		.setQueueFamilyIndex(graphicsQueueFamilyIndex)
		.setQueueCount(1)
		.setPQueuePriorities(queue_priorities);

	std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	auto const createInfo = vk::DeviceCreateInfo()
		.setQueueCreateInfoCount(1)
		.setPQueueCreateInfos(&queueInfo)
		.setEnabledExtensionCount(deviceExtensions.size())
		.setPpEnabledExtensionNames(deviceExtensions.data());

	_device = EnDevice::Create(gpu, createInfo);
	gpu.getMemoryProperties(&_device->memoryProperties);
	_device->getQueue(graphicsQueueFamilyIndex, 0, &graphicsQueue);
	presentQueue = graphicsQueue;
}

void Renderer::GetCapabilities() {
	uint32_t pSurfaceFormatCount;
	VK_CHECK_RESULT(this->gpu.getSurfaceFormatsKHR(this->surface, &pSurfaceFormatCount, NULL));
	std::vector<vk::SurfaceFormatKHR> pSurfaceFormats(pSurfaceFormatCount);
	VK_CHECK_RESULT(this->gpu.getSurfaceFormatsKHR(this->surface, &pSurfaceFormatCount, pSurfaceFormats.data()));

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
	this->gpu.getSurfaceCapabilitiesKHR(this->surface, &surfaceCapabilities);

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

	auto const cmdPoolInfo = vk::CommandPoolCreateInfo().setQueueFamilyIndex(graphicsQueueFamilyIndex);
	VK_CHECK_RESULT(_device->createCommandPool(&cmdPoolInfo, nullptr, &_commandPool));

	auto const cmd = vk::CommandBufferAllocateInfo()
		.setCommandPool(_commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);

	_draw.resize(swapchain.view.size());
	for (uint32_t i = 0; i < swapchain.view.size(); ++i) {
		VK_CHECK_RESULT(_device->allocateCommandBuffers(&cmd, &_draw[i]));
	}

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
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _shader->GetPipeline());
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _shader->GetPipelineLayout(), 0, 0, nullptr, 0, nullptr);

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
