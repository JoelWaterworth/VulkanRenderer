#pragma once
#include "Resource.h"
#include <vulkan/vulkan.h>
#include "../UniformInterface.h"
#include <experimental/filesystem>

using namespace std::experimental::filesystem;

class Texture : public Resource, public UniformInterface
{
public:
	Texture();
	static Texture* Create(
		Device* device,
		path p
		);
	static Texture* Create(
		Device* device,
		VkExtent2D extent,
		VkFormat format = VK_FORMAT_R16G16B16_SFLOAT,
		VkImageUsageFlags _usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VkSampler sampler = nullptr,
		VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	virtual void destroy(Device* device);
	virtual void bindMemory(Device* device, VkDeviceMemory memory, uint64_t localOffset);
	inline VkImageView getImageView() const { return _imageView; }
	inline VkImage getImage() const { return _image; }
	inline VkFormat getFormat() const { return _format; }
	inline VkImageLayout getImageLayout() const { return _layout; }
	inline VkExtent2D getResloution() const { return _extent; }
	virtual VkDescriptorType getDescriptorType();
	virtual VkDescriptorImageInfo* getImageInfo();
	void setImageLayout(VkCommandBuffer cmd, VkImageAspectFlags ImageAspects, VkImageLayout newImageLayout, VkImageSubresourceRange subResource);

private:
	bool bSampler = false;
	VkImage _image = nullptr;
	VkImageView _imageView = nullptr;
	VkSampler _sampler = nullptr;
	VkImageUsageFlags _usage;
	VkFormat _format;
	VkExtent2D _extent;
	VkImageLayout _layout;
	VkDescriptorImageInfo _descriptor;
	VkImageSubresourceRange subResource;
};