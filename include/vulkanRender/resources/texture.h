#pragma once
#include "resource.h"
#include <vulkan/vulkan.h>
#include "../uniformInterface.h"
#include <experimental/filesystem>

using namespace std::experimental::filesystem;

class Texture : public Resource, public UniformInterface
{
public:
	Texture();

	static Texture CreateCubeMap(
		Device* device,
		path p,
		VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT
	);

	static Texture Create(
		Device* device,
		path p,
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
		uint32_t mipLevels = 1,
		uint32_t levels = 1,
		bool bIsCubeMap = false
		);
	static Texture CreateFromDim(
		Device* device,
		int32_t dim,
		uint32_t mipLevels = 1,
		uint32_t levels = 1,
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
		bool bIsCubeMap = false
	);
	static Texture CreateBody(
		Device* device,
		VkExtent2D extent,
		VkFormat format = VK_FORMAT_R16G16B16_SFLOAT,
		VkImageUsageFlags _usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VkSampler sampler = nullptr,
		VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		uint32_t mipLevels = 1,
		uint32_t levels = 1,
		bool bIsCubeMap = false);

	VkSampler _sampler = VK_NULL_HANDLE;

private:
	VkImage _image = nullptr;
	VkImageView _imageView = nullptr;
	VkImageUsageFlags _usage;
	VkFormat _format;
	VkExtent2D _extent;
	VkDescriptorImageInfo _descriptor;
	VkImageSubresourceRange subResource;

public:
	virtual void destroy(Device* device);
	virtual void bindMemory(Device* device, VkDeviceMemory memory, uint64_t localOffset);
	void copyImage(VkCommandBuffer cmd, Texture* srcTexture, uint32_t regionCount, VkImageCopy* copyRegion);
	inline VkImageView getImageView() const { return _imageView; }
	inline VkImage getImage() const { return _image; }
	inline VkFormat getFormat() const { return _format; }
	inline VkImageLayout getImageLayout() const { return _descriptor.imageLayout; }
	inline VkExtent2D getResloution() const { return _extent; }
	inline VkImageSubresourceRange getSubResource() const { return subResource; }
	virtual VkDescriptorType getDescriptorType();
	virtual VkDescriptorImageInfo* getImageInfo();
	void setImageLayout(VkCommandBuffer cmd, VkImageAspectFlags ImageAspects, VkImageLayout newImageLayout, VkImageSubresourceRange subResource);
};
