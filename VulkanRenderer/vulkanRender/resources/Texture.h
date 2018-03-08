#pragma once
#include "Resource.h"
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include "../UniformInterface.h"

class Texture : public Resource, public UniformInterface
{
public:
	Texture();
	~Texture();
	static Texture* Create(
		EnDevice* device,
		vk::Extent2D extent,
		vk::Format format = vk::Format::eR16G16B16Sfloat,
		vk::ImageUsageFlags _usage = vk::ImageUsageFlagBits::eColorAttachment,
		vk::ImageLayout imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		vk::Sampler* sampler = nullptr);
	virtual void bindMemory(vk::DeviceMemory memory);
	virtual bool postAllocation();
	inline vk::ImageView getImageView() const { return _imageView; }
	inline vk::Image getImage() const { return _image; }
	inline vk::Format getFormat() const { return _format; }
	inline vk::ImageLayout getImageLayout() const { return _layout; }
	inline vk::Extent2D getResloution() const { return _extent; }

private:
	vk::Image _image = nullptr;
	vk::ImageView _imageView = nullptr;
	vk::Sampler* _sampler = nullptr;
	vk::ImageUsageFlags _usage;
	vk::Format _format;
	vk::Extent2D _extent;
	vk::ImageLayout _layout;
};