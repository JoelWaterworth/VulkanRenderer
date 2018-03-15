#include "Texture.h"
#include "../util.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture()
{
}

Texture * Texture::Create(EnDevice * device, path p)
{
	int width, height, nrChannels;
	unsigned char *data = stbi_load(p.string().c_str(), &width, &height, &nrChannels, 0);
	auto const samplerInfo = vk::SamplerCreateInfo()
		.setMagFilter(vk::Filter::eNearest)
		.setMinFilter(vk::Filter::eNearest)
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
		.setMipLodBias(0.0f)
		.setMinLod(0.0f)
		.setMaxLod(1.0f)
		.setAnisotropyEnable(0)
		.setMaxAnisotropy(1.0f)
		.setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
		.setCompareEnable(0)
		.setCompareOp(vk::CompareOp::eNever)
		.setUnnormalizedCoordinates(0);

	Texture* t = Texture::Create(
		device, vk::Extent2D(width, height),
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::ImageLayout::eColorAttachmentOptimal,
		device->createSampler(samplerInfo));
	void* ptr = device->mapMemory(t->memory, t->_offset, t->size);
	memcpy(ptr, data, t->size);
	device->unmapMemory(t->memory);
	return nullptr;
}

Texture* Texture::Create(EnDevice* device, vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usage, vk::ImageLayout imageLayout, vk::Sampler sampler) {
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
	texture->size = texture->requirments.size;
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
	texture->_descriptor = vk::DescriptorImageInfo(sampler, texture->_imageView, texture->_layout);
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

vk::DescriptorType Texture::getDescriptorType()
{
	return vk::DescriptorType::eCombinedImageSampler;
}

vk::DescriptorImageInfo * Texture::getImageInfo()
{
	return &_descriptor;
}
