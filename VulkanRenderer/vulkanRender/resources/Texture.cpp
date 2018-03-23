#include "Texture.h"
#include "../util.h"
#include "EnBuffer.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture()
{
}

Texture * Texture::Create(Device * device, path p)
{
	int width, height, nrChannels;
	unsigned char *data = stbi_load(p.string().c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
	VkDeviceSize size = width * height * 4;
	std::pair<VkBuffer, VkDeviceMemory> staging = device->allocateBuffer(
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* ptr = nullptr;
	vkMapMemory(device->handle(),staging.second, 0, size, 0, &ptr);
	memcpy(ptr, data, size);
	vkUnmapMemory(device->handle(), staging.second);
	stbi_image_free(data);
	
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_NEAREST;
	samplerInfo.minFilter = VK_FILTER_NEAREST;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.anisotropyEnable = 0;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.compareEnable = 0;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.unnormalizedCoordinates = 0;
	VkSampler sampler = nullptr;
	vkCreateSampler(device->handle(), &samplerInfo, nullptr, &sampler);

	Texture* t = Texture::Create(
		device, { (uint32_t)width, (uint32_t)height },
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		sampler);

	t->bSampler = true;
	VkCommandBuffer copycmd;
	auto const info = VkCommandBufferBeginInfo();
	device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &copycmd);
	vkBeginCommandBuffer(copycmd, &info);
	t->setImageLayout(copycmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, t->subResource);
	VkBufferImageCopy bufferImageCopy = {};
	bufferImageCopy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	bufferImageCopy.imageExtent = { (uint32_t)width, (uint32_t)height, 1 };
	vkCmdCopyBufferToImage(copycmd, staging.first, t->_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (uint32_t)1, &bufferImageCopy);

	t->setImageLayout(copycmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, t->subResource);

	device->submitCommandBuffer(copycmd, true);

	t->_descriptor = { t->_sampler, t->_imageView, VK_IMAGE_LAYOUT_GENERAL };
	vkDestroyBuffer(device->handle(), staging.first, nullptr);
	vkFreeMemory(device->handle(), staging.second, nullptr);
	return t;
}

Texture* Texture::Create(Device* device, VkExtent2D extent, VkFormat format, VkImageUsageFlags usage, VkImageLayout imageLayout, VkSampler sampler, VkMemoryPropertyFlags memoryProperties) {
	Texture* texture = new Texture();
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = format;
	imageInfo.extent = { extent.width, extent.height, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = 0;
	VK_CHECK_RESULT(vkCreateImage(device->handle(), &imageInfo, NULL, &texture->_image));
	vkGetImageMemoryRequirements(device->handle(), texture->_image, &texture->requirments);
	texture->size = texture->requirments.size;
	texture->_usage = usage;
	texture->_format = format;
	texture->_sampler = sampler;
	texture->_extent = extent;
	texture->_layout = imageLayout;
	device->attachResource(texture, memoryProperties);
	VkImageAspectFlags aspect;
	if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
		aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	else {
		aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
	};
	texture->subResource = {
		aspect, 0, 1, 0, 1
	};
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	viewInfo.subresourceRange = texture->subResource;
	viewInfo.image = texture->_image;
	vkCreateImageView(device->handle(), &viewInfo, nullptr, &texture->_imageView);
	texture->_descriptor = { sampler, texture->_imageView, texture->_layout };
	return texture;
}

void Texture::destroy(Device * device)
{
	vkDestroyImage(device->handle(), _image, nullptr);
	vkDestroyImageView(device->handle(), _imageView, nullptr);
	if (bSampler) {
		vkDestroySampler(device->handle(), _sampler, nullptr);
	}
}

void Texture::bindMemory(Device* device, VkDeviceMemory memory, uint64_t localOffset)
{
	vkBindImageMemory(device->handle(), _image, memory, _offset + localOffset);
}

VkDescriptorType Texture::getDescriptorType()
{
	return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
}

VkDescriptorImageInfo * Texture::getImageInfo()
{
	return &_descriptor;
}

void Texture::setImageLayout(VkCommandBuffer cmd, VkImageAspectFlags ImageAspects, VkImageLayout newImageLayout, VkImageSubresourceRange subResource)
{
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.oldLayout = _layout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = _image;
	imageMemoryBarrier.subresourceRange = subResource;

	VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	switch (_layout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:
			imageMemoryBarrier.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
	}

	switch (newImageLayout) {
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL :
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			destStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			destStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
	}

	vkCmdPipelineBarrier(cmd, srcStageFlags, destStageFlags, VkDependencyFlags(), 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	_layout = newImageLayout;
}
