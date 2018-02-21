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

Renderer::Renderer(std::string title, Window* window)
{
	initInstance(title);
	surface = window->createSurface(instance);
	initDevice();
	GetCapabilities();
	CreateSwapchain();
	std::vector<AttachmentInfo> colourInfo = { { capabilities.format.format, vk::ImageUsageFlagBits::eColorAttachment, vk::ImageLayout::ePresentSrcKHR, 1 }};
	AttachmentInfo depthInfo = { vk::Format::eD16Unorm, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal, 1 };
	renderTarget = new RenderTarget(device, capabilities.capabilities.maxImageExtent, colourInfo, depthInfo, &swapchain.view);
	resourceManger = new ResourceManger();
	mesh = Mesh::Create(&device, path("assets/Mesh/monkey.obj"));
	Shader::Create(&device, path("assets/shaders/Test.frag"), path("assets/shaders/Test.vert"));
}

Renderer::~Renderer()
{
	delete mesh;
	delete resourceManger;
	delete renderTarget;
	device.destroySwapchainKHR(swapchain.handle);
	instance.destroySurfaceKHR(surface);
	device.destroy();
	instance.destroy();
}

void Renderer::initInstance(std::string title)
{
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

void Renderer::initDebug()
{
	CreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(instance.getProcAddr("vkCreateDebugReportCallbackEXT"));
	CreateDebugReportCallbackEXT(this->instance, reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT*>(&dbgCreateInfo), NULL, &debugReport);
}

void Renderer::initDevice()
{
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
	uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
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

	VK_CHECK_RESULT(gpu.createDevice(&createInfo, NULL, &device));
	gpu.getMemoryProperties(&device.memoryProperties);
}

void Renderer::GetCapabilities()
{
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

void Renderer::CreateSwapchain()
{
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

	VK_CHECK_RESULT(this->device.createSwapchainKHR(&createInfo, NULL, &this->swapchain.handle));

	uint32_t imageCount;
	this->device.getSwapchainImagesKHR(this->swapchain.handle, &imageCount, NULL);
	this->swapchain.images.resize(imageCount);
	this->swapchain.view.resize(imageCount);
	this->device.getSwapchainImagesKHR(this->swapchain.handle, &imageCount, this->swapchain.images.data());
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
		VK_CHECK_RESULT(this->device.createImageView(&viewInfo, NULL, &this->swapchain.view[i]));
	}
}
