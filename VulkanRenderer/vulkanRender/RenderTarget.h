#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include <vector>
#include <utility>
#include "EnDevice.h"
#include "resources\Texture.h"

struct AttachmentInfo {
	vk::Format format;
	vk::ImageUsageFlags usage;
	vk::ImageLayout imageLayout;
	uint32_t layerCount;
};

class Attachment
{
public:
	Attachment();
	Attachment(vk::Image img,
		vk::Format fmat,
		vk::ImageUsageFlags u,
		vk::DescriptorImageInfo desInfo): 
			image(img),
			format(fmat),
			usage(u),
			descriptor(desInfo){};
	static std::pair<std::vector<Attachment>, vk::DeviceMemory> createAttachement(EnDevice* device, vk::Extent2D extent, vk::Sampler sampler, std::vector<AttachmentInfo> info);
private:
	vk::Image image;
	vk::Format format;
	vk::ImageUsageFlags usage;
	vk::DescriptorImageInfo descriptor;
public:
	inline vk::Image getImage() { return image; };
	inline vk::Format getFormat() { return format; };
	inline vk::ImageUsageFlags getUsage() { return usage; };
	inline vk::DescriptorImageInfo getDescriptor() { return descriptor; };
};

class RenderTarget
{
public:
	RenderTarget();
	static RenderTarget* Create(
		EnDevice* device, 
		vk::Extent2D resloution, 
		AttachmentInfo* req,
		uint32_t attachmentNumber,
		std::vector<vk::ImageView>* framebuffers = nullptr
	);
	static RenderTarget* CreateFromTextures(EnDevice* device, std::vector<Texture*> attachments, std::vector<vk::ImageView>* frameBufferImageViews = nullptr);
	void SetUp(std::vector<vk::ImageView>* frameBufferImageViews = nullptr);
	~RenderTarget();

	inline std::vector<Texture*> getAttachments() const { return attachments; }
	inline vk::Extent2D getResolution() const { return resolution; };
	inline size_t getAttachmentNum() const { return attachments.size(); }
	inline vk::RenderPass getRenderPass() const { return renderPass; }
	inline std::vector<vk::Framebuffer> getFramebuffers() const { return framebuffers; }
private:
	vk::Extent2D resolution;
	std::vector<Texture*> attachments;
	std::vector<vk::Framebuffer> framebuffers;
	vk::DeviceMemory memory;
	EnDevice* _device = nullptr;
	vk::Sampler sampler;
	vk::RenderPass renderPass;
};