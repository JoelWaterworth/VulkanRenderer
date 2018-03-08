#include "Texture.h"
#include "../util.h"
#include <iostream>

Texture::Texture()
{
}


Texture::~Texture()
{
	_device->destroyImage(_image);
	_device->destroyImageView(_imageView);
}

Texture* Texture::Create(EnDevice* device, vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usage, vk::ImageLayout imageLayout, vk::Sampler* sampler) {
	Texture* texture = new Texture();
	auto const imageInfo = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(format)
		.setExtent(vk::Extent3D(extent.width, extent.height, 1))
		.setMipLevels(1)
		.setArrayLayers(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(usage | vk::ImageUsageFlagBits::eSampled)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setQueueFamilyIndexCount(0);
	VK_CHECK_RESULT(device->createImage(&imageInfo, NULL, &texture->_image));
	texture->requirments = device->getImageMemoryRequirements(texture->_image);
	texture->_usage = usage;
	texture->_format = format;
	texture->_sampler = sampler;
	texture->_device = device;
	texture->_extent = extent;
	texture->_layout = imageLayout;
	return texture;
}

void Texture::bindMemory(vk::DeviceMemory memory)
{
	_device->bindImageMemory(_image, memory, _offset);
}

bool Texture::postAllocation()
{
	vk::ImageAspectFlagBits aspect;
	if (_usage == vk::ImageUsageFlagBits::eColorAttachment) {
		aspect = vk::ImageAspectFlagBits::eColor;
	}
	else {
		aspect = vk::ImageAspectFlagBits::eDepth;
	};

	auto const viewInfo = vk::ImageViewCreateInfo()
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(_format)
		.setComponents(vk::ComponentMapping(
			vk::ComponentSwizzle::eIdentity
		))
		.setSubresourceRange(vk::ImageSubresourceRange(
			aspect, 0, 1, 0, 1
		))
		.setImage(_image);
	_imageView = _device->createImageView(viewInfo);
	return true;
}