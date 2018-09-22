#include "renderTarget.h"
#include "util.h"
#include "resources/resource.h"
#include "resources/texture.h"
#include <iostream>
#include <functional>
#include <numeric>


RenderTarget::RenderTarget()
{
}

RenderTarget RenderTarget::Create(Device* device, VkExtent2D r, AttachmentInfo* req, uint32_t attachmentCount, std::vector<VkImageView>* frameBufferImageViews) {
	RenderTarget rt = RenderTarget();
	rt._resolution = r;
	if (attachmentCount < 2) {
		assert("colourReq cannot be empty");
	}

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
	VK_CHECK_RESULT(vkCreateSampler(device->handle(), &samplerInfo, nullptr, &rt._sampler));

	rt._attachments.resize(attachmentCount);
	for (int i = 0; i < attachmentCount; i++) {
		rt._attachments[i] = Texture::CreateBody(device, r, req[i].format, req[i].usage, req[i].imageLayout, rt._sampler, false);
	}
	rt.SetUp(device, frameBufferImageViews);
	return rt;
}

RenderTarget RenderTarget::Create(Device* device, VkExtent2D r, AttachmentInfo* req, uint32_t attachmentCount, std::vector<Texture> colourAttachments, std::vector<VkImageView>* frameBufferImageViews) {
	RenderTarget rt = RenderTarget();
	rt._resolution = r;
	if (attachmentCount < 2) {
		assert("colourReq cannot be empty");
	}

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
	VK_CHECK_RESULT(vkCreateSampler(device->handle(), &samplerInfo, nullptr, &rt._sampler));

	rt._attachments.resize(attachmentCount);
	for (int i = 0; i < attachmentCount; i++) {
		if (i == (attachmentCount - 1)) {
			rt._attachments[i] = Texture::CreateBody(device, r, req[i].format, req[i].usage, req[i].imageLayout, rt._sampler);
		}
		else {
			rt._attachments[i] = colourAttachments[i];
		}
	}
	rt.SetUp(device, frameBufferImageViews);
	return rt;
}

void RenderTarget::SetUp(Device* device, std::vector<VkImageView>* frameBufferImageViews) {
	std::vector<VkAttachmentDescription> renderpassAttachments(_attachments.size());
	for (int i = 0; i < _attachments.size(); i++) {
		renderpassAttachments[i].format = _attachments[i].getFormat();
		renderpassAttachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
		renderpassAttachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		renderpassAttachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		renderpassAttachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		renderpassAttachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		renderpassAttachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		renderpassAttachments[i].finalLayout = _attachments[i].getImageLayout();
	}

	uint32_t colourAttachmentCount = _attachments.size() - 1;

	std::vector<VkAttachmentReference> colorAttachmentsRef(colourAttachmentCount);
	for (int i = 0; i < colourAttachmentCount; i++) {
		colorAttachmentsRef[i].attachment = i;
		colorAttachmentsRef[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	VkAttachmentReference depthAttachmentsRef = {};
	depthAttachmentsRef.attachment = colourAttachmentCount;
	depthAttachmentsRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.colorAttachmentCount = colorAttachmentsRef.size();
	subpass.pColorAttachments = colorAttachmentsRef.data();
	subpass.pDepthStencilAttachment = &depthAttachmentsRef;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkSubpassDependency dependencies[2] = {};
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo deferredRenderPassCreateInfo = {};
	deferredRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	deferredRenderPassCreateInfo.attachmentCount = renderpassAttachments.size();
	deferredRenderPassCreateInfo.pAttachments = renderpassAttachments.data();
	deferredRenderPassCreateInfo.subpassCount = 1;
	deferredRenderPassCreateInfo.pSubpasses = &subpass;
	deferredRenderPassCreateInfo.dependencyCount = 2;
	deferredRenderPassCreateInfo.pDependencies = dependencies;
	VK_CHECK_RESULT(vkCreateRenderPass(device->handle(), &deferredRenderPassCreateInfo, nullptr, &this->_renderPass));

	if (frameBufferImageViews) {
		for (auto& present_image_view : *frameBufferImageViews) {
			VkImageView framebufferAttachments[2] = { present_image_view, _attachments[_attachments.size()-1].getImageView() };
			VkFramebufferCreateInfo frameBufferCreateInfo = {};
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCreateInfo.renderPass = _renderPass;
			frameBufferCreateInfo.attachmentCount = 2;
			frameBufferCreateInfo.pAttachments = framebufferAttachments;
			frameBufferCreateInfo.width = _resolution.width;
			frameBufferCreateInfo.height = _resolution.height;
			frameBufferCreateInfo.layers = 1;
			VkFramebuffer frame = nullptr;
			vkCreateFramebuffer(device->handle(), &frameBufferCreateInfo, NULL, &frame);
			this->_framebuffers.push_back(frame);
		}
	} else {
		std::vector<VkImageView> attachmentsViews(_attachments.size());
		for (int i = 0; i < attachmentsViews.size(); i++) {
			attachmentsViews[i] = _attachments[i].getImageView();
		}

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.renderPass = _renderPass;
		frameBufferCreateInfo.attachmentCount = attachmentsViews.size();
		frameBufferCreateInfo.pAttachments = attachmentsViews.data();
		frameBufferCreateInfo.width = _resolution.width;
		frameBufferCreateInfo.height = _resolution.height;
		frameBufferCreateInfo.layers = 1;
		VkFramebuffer frame = VK_NULL_HANDLE;
		vkCreateFramebuffer(device->handle(), &frameBufferCreateInfo, NULL, &frame);
		_framebuffers.push_back(frame);
	}
}

void RenderTarget::destroy(Device * device) {
	for (VkFramebuffer framebuffer : _framebuffers) {
		vkDestroyFramebuffer(device->handle(), framebuffer, nullptr);
	}

	vkDestroyRenderPass(device->handle(), _renderPass, nullptr);
	if (_sampler != VK_NULL_HANDLE) {
		vkDestroySampler(device->handle(), _sampler, nullptr);
	}

	for (auto attachment : _attachments) {
		attachment.destroy(device);
	}
}

std::vector<Texture> RenderTarget::takeAttachments(Device * device, std::vector<uint8_t> indices) {
	std::vector<Texture> textures = std::vector<Texture>();
	for (uint8_t index : indices) {
		_attachments[index].bIsSamplerOwned = true;
		textures.push_back(_attachments[index]);
		_attachments.erase(_attachments.begin() + index);
	}
	_sampler = VK_NULL_HANDLE;
	destroy(device);
	return textures;
}

Texture RenderTarget::takeAttachment(Device * device, uint8_t index) {
	Texture texture = _attachments[index];
	_attachments.erase(_attachments.begin() + index);
	texture.bIsSamplerOwned = true;
	_sampler = VK_NULL_HANDLE;
	destroy(device);
	return texture;
}

RenderTarget RenderTarget::CreateFromTextures(Device * device, std::vector<Texture> attachments, std::vector<VkImageView>* frameBufferImageViews) {
	RenderTarget rt = RenderTarget();
	rt._resolution = attachments[0].getResloution();
	rt._attachments = attachments;
	rt.SetUp(device, frameBufferImageViews);
	return rt;
}
