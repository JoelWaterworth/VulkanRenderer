#include "RenderTarget.h"
#include "util.h"
#include "resources\Resource.h"
#include "resources\Texture.h"
#include <iostream>
#include <functional>
#include <numeric>


RenderTarget::RenderTarget()
{
}

RenderTarget* RenderTarget::Create(EnDevice* d, vk::Extent2D r, AttachmentInfo* req, uint32_t attachmentCount, std::vector<vk::ImageView>* frameBufferImageViews)
{
	RenderTarget* rt = new RenderTarget();
	rt->resolution = r;
	rt->_device = d;
	if (attachmentCount < 2) {
		assert("colourReq cannot be empty");
	}

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
	rt->sampler = d->createSampler(samplerInfo);
	rt->attachments.resize(attachmentCount);
	for (int i = 0; i < attachmentCount; i++) {
		rt->attachments[i] = Texture::Create(d, r, req[i].format, req[i].usage, req[i].imageLayout, rt->sampler);
	}
	rt->SetUp(frameBufferImageViews);
	return rt;
}

void RenderTarget::SetUp(std::vector<vk::ImageView>* frameBufferImageViews)
{
	std::vector<vk::AttachmentDescription> renderpassAttachments(attachments.size());
	for (int i = 0; i < attachments.size(); i++) {
		renderpassAttachments[i] = vk::AttachmentDescription()
			.setFormat(attachments[i]->getFormat())
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(attachments[i]->getImageLayout());
	}

	uint32_t colourAttachmentCount = attachments.size() - 1;

	std::vector<vk::AttachmentReference> colorAttachmentsRef(colourAttachmentCount);
	for (int i = 0; i < colourAttachmentCount; i++) {
		colorAttachmentsRef[i] = vk::AttachmentReference()
			.setAttachment(i)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	}
	auto const depthAttachmentsRef = vk::AttachmentReference()
		.setAttachment(colourAttachmentCount)
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	auto const subpass = vk::SubpassDescription()
            .setColorAttachmentCount(colorAttachmentsRef.size())
            .setPColorAttachments(colorAttachmentsRef.data())
            .setPDepthStencilAttachment(&depthAttachmentsRef)
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	vk::SubpassDependency dependencies[2] = {
		vk::SubpassDependency()
			.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
			.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite),
		vk::SubpassDependency()
			.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
			.setDstSubpass(VK_SUBPASS_EXTERNAL)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
	};
	auto const deferredRenderPassCreateInfo = vk::RenderPassCreateInfo()
            .setAttachmentCount(renderpassAttachments.size())
            .setPAttachments(renderpassAttachments.data())
            .setSubpassCount(1)
            .setPSubpasses(&subpass)
            .setDependencyCount(2)
            .setPDependencies(dependencies);
	this->renderPass = _device->createRenderPass(deferredRenderPassCreateInfo);

	if (frameBufferImageViews) {
		for (auto& present_image_view : *frameBufferImageViews) {
			vk::ImageView framebufferAttachments[2] = { present_image_view, attachments[attachments.size()-1]->getImageView() };
			auto const frameBufferCreateInfo = vk::FramebufferCreateInfo()
				.setRenderPass(renderPass)
				.setAttachmentCount(2)
				.setPAttachments(framebufferAttachments)
				.setWidth(resolution.width)
				.setHeight(resolution.height)
				.setLayers(1);
			this->framebuffers.push_back(_device->createFramebuffer(frameBufferCreateInfo));
		}
	} else {
		std::vector<vk::ImageView> attachmentsViews(attachments.size());
		for (int i = 0; i < attachments.size(); i++) {
			attachmentsViews[i] = attachments[i]->getImageView();
		}

		auto const frameBufferCreateInfo = vk::FramebufferCreateInfo()
			.setRenderPass(renderPass)
			.setAttachmentCount(attachmentsViews.size())
			.setPAttachments(attachmentsViews.data())
			.setWidth(resolution.width)
			.setHeight(resolution.height)
			.setLayers(1);
		this->framebuffers.push_back(_device->createFramebuffer(frameBufferCreateInfo));
		attachments.pop_back();
	}
}

RenderTarget* RenderTarget::CreateFromTextures(EnDevice * device, std::vector<Texture*> attachments, std::vector<vk::ImageView>* frameBufferImageViews)
{
	RenderTarget* rt = new RenderTarget();
	rt->resolution = attachments[0]->getResloution();
	rt->_device = device;
	rt->attachments = attachments;
	rt->SetUp(frameBufferImageViews);
	return rt;
}

RenderTarget::~RenderTarget() {
	for (vk::Framebuffer framebuffer : framebuffers) {
		_device->destroyFramebuffer(framebuffer);
	}

	_device->destroyRenderPass(renderPass);
	_device->destroySampler(sampler);

	for (auto attachment : attachments) {
		attachment->destroy(_device);
		delete attachment;
	}
}

Attachment::Attachment()
{
}

std::pair<std::vector<Attachment>, vk::DeviceMemory> Attachment::createAttachement(EnDevice* device, vk::Extent2D extent, vk::Sampler sampler, std::vector<AttachmentInfo>info)
{
	std::pair<std::vector<Attachment>, vk::DeviceMemory> pair;
	std::vector<std::pair<vk::Image, vk::MemoryRequirements>> images(info.size());
	vk::DeviceSize size = 0;

	for (int i = 0; i < info.size(); i++) {
		auto const imageInfo = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setFormat(info[i].format)
			.setExtent(vk::Extent3D(extent.width, extent.height, 1))
			.setMipLevels(1)
			.setArrayLayers(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(info[i].usage | vk::ImageUsageFlagBits::eSampled)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setQueueFamilyIndexCount(0);
		vk::Image image = nullptr;
		VK_CHECK_RESULT(device->createImage(&imageInfo, NULL, &image));
		auto req = device->getImageMemoryRequirements(image);
		size += req.size;
		images[i] = std::make_pair(image, req);
	}

	auto memAlloc = vk::MemoryAllocateInfo()
		.setAllocationSize(size)
		.setMemoryTypeIndex(0);
	if (!device->memoryTypeFromProperties(
		images[0].second,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		&memAlloc.memoryTypeIndex)) {
		assert("no suitable memory type");
	};

	pair.second = device->allocateMemory(memAlloc);
	for (int i = 0; i < info.size(); i++) {
		int size = 0;
		for (int ii = 0; ii < i; ii++) {
			size += images[ii].second.size;
		}
		device->bindImageMemory(images[i].first, pair.second, size);
	}
	std::vector<Attachment> attachments(info.size());
	for (int i = 0; i < info.size(); i++) {
		vk::ImageAspectFlagBits aspect;
		if (info[i].usage == vk::ImageUsageFlagBits::eColorAttachment) {
			aspect = vk::ImageAspectFlagBits::eColor;
		}
		else {
			aspect = vk::ImageAspectFlagBits::eDepth;
		};

		auto const viewInfo = vk::ImageViewCreateInfo()
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(info[i].format)
			.setComponents(vk::ComponentMapping(
				vk::ComponentSwizzle::eIdentity
			))
			.setSubresourceRange(vk::ImageSubresourceRange(
				aspect, 0, 1, 0, 1
			))
			.setImage(images[i].first);
		vk::ImageView imageView = device->createImageView(viewInfo);
		vk::Image image;
			attachments[i] = Attachment(
				images[i].first, 
				info[i].format,
				info[i].usage, 
				vk::DescriptorImageInfo()
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.setImageView(imageView)
					.setSampler(sampler)
			);
		
	}
	pair.first = attachments;
	return pair;
}
