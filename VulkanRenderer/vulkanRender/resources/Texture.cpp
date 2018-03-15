#include "Texture.h"
#include "../util.h"
#include "EnBuffer.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture()
{
}

Texture * Texture::Create(EnDevice * device, path p)
{
	int width, height, nrChannels;
	unsigned char *data = stbi_load(p.string().c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
	vk::DeviceSize size = width * height * 4;
	std::pair<vk::Buffer, vk::DeviceMemory> staging = device->allocateBuffer(
		size,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	void* ptr = device->mapMemory(staging.second, 0, size);
	memcpy(ptr, data, size);
	device->unmapMemory(staging.second);
	stbi_image_free(data);
	
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
		vk::Format::eB8G8R8A8Unorm,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
		vk::ImageLayout::eUndefined,
		device->createSampler(samplerInfo),
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	vk::CommandBuffer copycmd;
	auto const info = vk::CommandBufferBeginInfo();
	device->createCommandBuffer(vk::CommandBufferLevel::ePrimary, 1, &copycmd);
	copycmd.begin(info);
	//t->setImageLayout(copycmd, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eTransferDstOptimal, t->subResource);
	auto const bufferImageCopy = vk::BufferImageCopy()
		.setImageSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
		.setImageExtent(vk::Extent3D(width, height, 1))
		.setBufferOffset(0);
	copycmd.copyBufferToImage(staging.first, t->_image, vk::ImageLayout::eTransferDstOptimal, (uint32_t)1, &bufferImageCopy);

	t->setImageLayout(copycmd, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eShaderReadOnlyOptimal, t->subResource);

	device->submitCommandBuffer(copycmd, true);

	t->_descriptor = vk::DescriptorImageInfo(t->_sampler, t->_imageView, vk::ImageLayout::eGeneral);
	device->destroyBuffer(staging.first);
	device->freeMemory(staging.second);
	VkImage image = *reinterpret_cast<VkImage*>(&t->_image);
	device->setObjectName((uint64_t)image, vk::DebugReportObjectTypeEXT::eImage, "texture");
	return t;
}

Texture* Texture::Create(EnDevice* device, vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usage, vk::ImageLayout imageLayout, vk::Sampler sampler, vk::MemoryPropertyFlags memoryProperties) {
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
	device->attachResource(texture, memoryProperties);
	vk::ImageAspectFlagBits aspect;
	if (usage & vk::ImageUsageFlagBits::eColorAttachment) {
		aspect = vk::ImageAspectFlagBits::eColor;
	}
	else {
		aspect = vk::ImageAspectFlagBits::eDepth;
	};
	texture->subResource = vk::ImageSubresourceRange(
		aspect, 0, 1, 0, 1
	);
	auto const viewInfo = vk::ImageViewCreateInfo()
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(format)
		.setComponents(vk::ComponentMapping(
			vk::ComponentSwizzle::eIdentity
		))
		.setSubresourceRange(texture->subResource)
		.setImage(texture->_image);
	texture->_imageView = device->createImageView(viewInfo);
	texture->_descriptor = vk::DescriptorImageInfo(sampler, texture->_imageView, texture->_layout);
	return texture;
}

void Texture::destroy(EnDevice * device)
{
	device->destroyImage(_image);
	device->destroyImageView(_imageView);
	if (_sampler) {
		device->destroySampler(_sampler);
	}
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

void Texture::setImageLayout(vk::CommandBuffer cmd, vk::ImageAspectFlags ImageAspects, vk::ImageLayout newImageLayout, vk::ImageSubresourceRange subResource)
{
	auto imageMemoryBarrier = vk::ImageMemoryBarrier()
		.setOldLayout(_layout)
		.setNewLayout(newImageLayout)
		.setImage(_image)
		.setSubresourceRange(subResource);

	vk::PipelineStageFlags srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
	vk::PipelineStageFlags destStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;

	switch (_layout) {
		case vk::ImageLayout::eUndefined:
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits(0);
			break;
		case vk::ImageLayout::ePreinitialized:
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
			break;
		case vk::ImageLayout::eTransferDstOptimal:
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
			break;
	}

	switch (newImageLayout) {
		case vk::ImageLayout::eTransferSrcOptimal:
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			break;
		case vk::ImageLayout::eTransferDstOptimal:
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
			destStageFlags = vk::PipelineStageFlagBits::eTransfer;
			break;
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			destStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
			break;
	}

	cmd.pipelineBarrier(srcStageFlags, destStageFlags, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	_layout = newImageLayout;
}
