#pragma once
#include <vulkan/vulkan.h>
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

	static RenderTarget Create(
		Device* device,
		VkExtent2D resloution,
		AttachmentInfo* req,
		uint32_t attachmentNumber,
		std::vector<VkImageView>* framebuffers = nullptr
	);

	static RenderTarget Create(
		Device* device,
		VkExtent2D resloution,
		AttachmentInfo* req,
		uint32_t attachmentNumber,
		std::vector<Texture> colourAttachments,
		std::vector<VkImageView>* framebuffers = nullptr
	);

	static RenderTarget CreateFromTextures(Device* device, std::vector<Texture> attachments, std::vector<VkImageView>* frameBufferImageViews = nullptr);
	void SetUp(Device* device, std::vector<VkImageView>* frameBufferImageViews = nullptr);
	void destroy(Device* device);
	std::vector<Texture> takeAttachments(Device * device, std::vector <uint8_t> indeices);
	Texture takeAttachment(Device * device, uint8_t index);
	inline std::vector<Texture> getAttachments() const { return _attachments; }
	inline VkExtent2D getResolution() const { return _resolution; };
	inline size_t getAttachmentNum() const { return _attachments.size(); }
	inline size_t getColourAttachmentNum() const { return _attachments.size() - 1; }
	inline VkRenderPass getRenderPass() const { return _renderPass; }
	inline std::vector<VkFramebuffer> getFramebuffers() const { return _framebuffers; }
private:
	VkExtent2D _resolution;
	std::vector<Texture> _attachments;
	std::vector<VkFramebuffer> _framebuffers;
	//VkDeviceMemory memory = VK_NULL_HANDLE;
	VkSampler _sampler = VK_NULL_HANDLE;
	VkRenderPass _renderPass = VK_NULL_HANDLE;
};
