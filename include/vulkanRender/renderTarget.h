#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include <vector>
#include <utility>
#include "device.h"
#include "resources/texture.h"

struct AttachmentInfo {
	VkFormat format;
	VkImageUsageFlags usage;
	VkImageLayout imageLayout;
	uint32_t layerCount;
};

class RenderTarget
{
public:
	RenderTarget();
	static RenderTarget* Create(
		Device* device,
		VkExtent2D resloution,
		AttachmentInfo* req,
		uint32_t attachmentNumber,
		std::vector<VkImageView>* framebuffers = nullptr
	);
	static RenderTarget* CreateFromTextures(Device* device, std::vector<Texture*> attachments, std::vector<VkImageView>* frameBufferImageViews = nullptr);
	void SetUp(std::vector<VkImageView>* frameBufferImageViews = nullptr);
	~RenderTarget();

	inline std::vector<Texture*> getAttachments() const { return attachments; }
	inline VkExtent2D getResolution() const { return resolution; };
	inline size_t getAttachmentNum() const { return attachments.size(); }
	inline size_t getColourAttachmentNum() const { return attachments.size() - 1; }
	inline VkRenderPass getRenderPass() const { return renderPass; }
	inline std::vector<VkFramebuffer> getFramebuffers() const { return framebuffers; }
private:
	VkExtent2D resolution;
	std::vector<Texture*> attachments;
	std::vector<VkFramebuffer> framebuffers;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	Device* _device = nullptr;
	VkSampler sampler = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;
};
