#pragma once
#include "Resource.h"
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include "../UniformInterface.h"
#include <experimental/filesystem>

using namespace std::experimental::filesystem;

class Texture : public Resource, public UniformInterface
{
public:
	Texture();
	static Texture* Create(
		EnDevice* device,
		path p
		);
	static Texture* Create(
		EnDevice* device,
		vk::Extent2D extent,
		vk::Format format = vk::Format::eR16G16B16Sfloat,
		vk::ImageUsageFlags _usage = vk::ImageUsageFlagBits::eColorAttachment,
		vk::ImageLayout imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		vk::Sampler sampler = nullptr,
		vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal);
	virtual void destroy(EnDevice* device);
	virtual void bindMemory(EnDevice* device, vk::DeviceMemory memory, uint64_t localOffset);
	inline vk::ImageView getImageView() const { return _imageView; }
	inline vk::Image getImage() const { return _image; }
	inline vk::Format getFormat() const { return _format; }
	inline vk::ImageLayout getImageLayout() const { return _layout; }
	inline vk::Extent2D getResloution() const { return _extent; }
	virtual vk::DescriptorType getDescriptorType();
	virtual vk::DescriptorImageInfo* getImageInfo();
	void setImageLayout(vk::CommandBuffer cmd, vk::ImageAspectFlags ImageAspects, vk::ImageLayout newImageLayout, vk::ImageSubresourceRange subResource);

private:
	vk::Image _image = nullptr;
	vk::ImageView _imageView = nullptr;
	vk::Sampler _sampler = nullptr;
	vk::ImageUsageFlags _usage;
	vk::Format _format;
	vk::Extent2D _extent;
	vk::ImageLayout _layout;
	vk::DescriptorImageInfo _descriptor;
	vk::ImageSubresourceRange subResource;
};