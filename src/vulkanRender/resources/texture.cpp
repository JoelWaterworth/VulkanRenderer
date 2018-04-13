#include "texture.h"
#include "../util.h"
#include "enBuffer.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <gli/gli.hpp>

Texture::Texture()
{
}

Texture * Texture::CreateCubeMap(Device * device, path p, VkFormat format)
{
	gli::texture_cube texCube(gli::load(p.string()));
	assert(!texCube.empty());

	uint32_t width = static_cast<uint32_t>(texCube.extent().x);
	uint32_t height = static_cast<uint32_t>(texCube.extent().y);
	uint32_t mipLevels = static_cast<uint32_t>(texCube.levels());

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

	Texture* t = Texture::CreateBody(
		device, { width, height },
		format,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		sampler,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		mipLevels,
		6,
		true
	);

	std::pair<VkBuffer, VkDeviceMemory> staging = device->allocateBuffer(
		texCube.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* ptr = nullptr;
	vkMapMemory(device->handle(), staging.second, 0, texCube.size(), 0, &ptr);
	memcpy(ptr, texCube.data(), texCube.size());
	vkUnmapMemory(device->handle(), staging.second);
//	stbi_image_free(data);

	t->bSampler = true;

	VkCommandBuffer copycmd;
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &copycmd);
	vkBeginCommandBuffer(copycmd, &info);

	std::vector<VkBufferImageCopy> bufferCopyRegions;
	size_t offset = 0;

	for (uint32_t face = 0; face < 6; face++)
	{
		for (uint32_t level = 0; level < mipLevels; level++)
		{
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = level;
			bufferCopyRegion.imageSubresource.baseArrayLayer = face;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(texCube[face][level].extent().x);
			bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(texCube[face][level].extent().y);
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);
			offset += texCube[face][level].size();
		}
	}

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = mipLevels;
	subresourceRange.layerCount = 6;

	t->setImageLayout(copycmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

	vkCmdCopyBufferToImage(copycmd, staging.first, t->_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (uint32_t)bufferCopyRegions.size(), bufferCopyRegions.data());

	t->setImageLayout(copycmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);

	device->submitCommandBuffer(copycmd, true);

	t->_descriptor = { t->_sampler, t->_imageView, VK_IMAGE_LAYOUT_GENERAL };
	vkDestroyBuffer(device->handle(), staging.first, nullptr);
	vkFreeMemory(device->handle(), staging.second, nullptr);
	return t;
}

Texture * Texture::Create(Device * device, path p, VkFormat format, uint32_t levels, uint32_t mipLevels, bool bIsCubeMap)
{
	int width, height, nrChannels = 0;
	unsigned char *data = stbi_load(p.string().c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
	VkDeviceSize size = width * height * 4;
	
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

	Texture* t = Texture::CreateBody(
		device, { (uint32_t)width, (uint32_t)height },
		format,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		sampler,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		mipLevels,
		levels,
		bIsCubeMap
	);
	
	std::pair<VkBuffer, VkDeviceMemory> staging = device->allocateBuffer(
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* ptr = nullptr;
	vkMapMemory(device->handle(), staging.second, 0, size, 0, &ptr);
	memcpy(ptr, data, size);
	vkUnmapMemory(device->handle(), staging.second);
	stbi_image_free(data);

	t->bSampler = true;
	VkCommandBuffer copycmd;
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
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

Texture * Texture::CreateFromDim(Device * device, int32_t dim, uint32_t levels, uint32_t mipLevels, VkFormat format, bool bIsCubeMap) {
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

	Texture* t = Texture::CreateBody(
		device, { (uint32_t)dim, (uint32_t)dim },
		format,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		sampler,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		mipLevels,
		levels,
		bIsCubeMap
	);
	return t;
}


Texture* Texture::CreateBody(Device* device, VkExtent2D extent, VkFormat format, VkImageUsageFlags usage, VkImageLayout imageLayout, VkSampler sampler, VkMemoryPropertyFlags memoryProperties, uint32_t mipLevels, uint32_t levels, bool bIsCubeMap) {
	Texture* texture = new Texture();
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = format;
	imageInfo.extent = { extent.width, extent.height, 1 };
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = levels;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = 0;
	if (bIsCubeMap) {
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}
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
		aspect, 0, mipLevels, 0, levels
	};
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	if (bIsCubeMap) {
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	}
	else {
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	}
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

void Texture::copyImage(VkCommandBuffer cmd, Texture* srcTexture, uint32_t regionCount, VkImageCopy* copyRegion) {
	vkCmdCopyImage(cmd, srcTexture->getImage(), srcTexture->getImageLayout(), _image, _layout, 1, copyRegion);
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
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
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
			destStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
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
