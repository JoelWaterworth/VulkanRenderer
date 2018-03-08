#include "Texture.h"
#include "../util.h"
#include <iostream>

Texture::Texture()
{
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
	texture->_extent = extent;
	texture->_layout = imageLayout;
	device->attachResource(texture, vk::MemoryPropertyFlagBits::eDeviceLocal);
	vk::ImageAspectFlagBits aspect;
	if (usage == vk::ImageUsageFlagBits::eColorAttachment) {
		aspect = vk::ImageAspectFlagBits::eColor;
	}
	else {
		aspect = vk::ImageAspectFlagBits::eDepth;
	};

	auto const viewInfo = vk::ImageViewCreateInfo()
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(format)
		.setComponents(vk::ComponentMapping(
			vk::ComponentSwizzle::eIdentity
		))
		.setSubresourceRange(vk::ImageSubresourceRange(
			aspect, 0, 1, 0, 1
		))
		.setImage(texture->_image);
	texture->_imageView = device->createImageView(viewInfo);
	return texture;
}

void Texture::destroy(EnDevice * device)
{
	device->destroyImage(_image);
	device->destroyImageView(_imageView);
}

void Texture::bindMemory(EnDevice* device, vk::DeviceMemory memory, uint64_t localOffset)
{
	device->bindImageMemory(_image, memory, _offset + localOffset);
}