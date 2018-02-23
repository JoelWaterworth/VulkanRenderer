#include "Shader.h"
#include "../util.h"
#include <iostream>
#include <fstream>

std::pair<char *, size_t> Shader::compile(path shader) {
	path spv_path = shader;
	std::string filename = shader.filename().string();
	filename.append("spv.spv");
	spv_path.replace_filename(filename);
	std::string command = "glslangValidator -V -o ";
	command.append(spv_path.string());
	command.push_back(' ');
	command.append(shader.string());
	system(command.data());
	FILE *fp;
	fopen_s(&fp, spv_path.string().data(), "rb");
	if (!fp) {
		return std::pair<char *, size_t>();
	}

	fseek(fp, 0L, SEEK_END);
	long int size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);

	void *shader_code = malloc(size);
	size_t retval = fread(shader_code, size, 1, fp);
	assert(retval == 1);

	fclose(fp);

	return std::make_pair((char *)shader_code, size);
}

char * Shader::readSpv(const char * filename, size_t * psize)
{
	FILE *fp;
	fopen_s(&fp, filename, "rb");
	if (!fp) {
		return nullptr;
	}

	fseek(fp, 0L, SEEK_END);
	long int size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);

	void *shader_code = malloc(size);
	size_t retval = fread(shader_code, size, 1, fp);
	assert(retval == 1);

	*psize = size;

	fclose(fp);

	return (char *)shader_code;
}

vk::ShaderModule Shader::createShaderModule(EnDevice* device, const void * code, size_t size)
{
	auto const moduleCreateInfo = vk::ShaderModuleCreateInfo().setCodeSize(size).setPCode((uint32_t const *)code);

	vk::ShaderModule module;
	auto result = device->createShaderModule(&moduleCreateInfo, nullptr, &module);
	assert(result == vk::Result::eSuccess);

	return module;
}

pair<vk::ShaderModule, vk::PipelineShaderStageCreateInfo> Shader::createPipelineStageInfo(EnDevice* device, path shader, vk::ShaderStageFlagBits stage)
{
	std::pair<char *, size_t> s = compile(shader);
	vk::ShaderModule Module = Shader::createShaderModule(device, s.first, s.second);
	vk::PipelineShaderStageCreateInfo pipelineStageInfo = vk::PipelineShaderStageCreateInfo()
		.setModule(Module)
		.setPName("main")
		.setStage(stage);
	return make_pair(Module, pipelineStageInfo);
}

Shader* Shader::Create(EnDevice * device, RenderTarget* renderTarget, path vertPath, path fragPath, vector<ShaderLayout> layouts)
{
	pair<vk::ShaderModule, vk::PipelineShaderStageCreateInfo> vertRes =
		Shader::createPipelineStageInfo(device, vertPath, vk::ShaderStageFlagBits::eVertex);
	pair<vk::ShaderModule, vk::PipelineShaderStageCreateInfo> fragRes =
		Shader::createPipelineStageInfo(device, fragPath, vk::ShaderStageFlagBits::eFragment);

	vk::PipelineShaderStageCreateInfo shaderStageCreateInfos[] = {
		vertRes.second, fragRes.second
	};

	vk::VertexInputBindingDescription const vertexInputBindingDescriptions[] = {
		vk::VertexInputBindingDescription(0, sizeof(float) * 3, vk::VertexInputRate::eVertex)
	};

	vk::VertexInputAttributeDescription vertexInputAttributeDescriptions[] = {
            vk::VertexInputAttributeDescription ()
			.setLocation(0)
			.setBinding(0)
			.setFormat(vk::Format::eR32G32B32A32Sfloat)
			.setOffset(0)
	};
	auto const vertexInputStateInfo = vk::PipelineVertexInputStateCreateInfo()
		.setVertexAttributeDescriptionCount(1)
		.setPVertexAttributeDescriptions(vertexInputAttributeDescriptions)
		.setVertexBindingDescriptionCount(1)
		.setPVertexBindingDescriptions(vertexInputBindingDescriptions);

	auto const vertexInputAssemblyStateInfo = vk::PipelineInputAssemblyStateCreateInfo()
		.setPrimitiveRestartEnable(0)
		.setTopology(vk::PrimitiveTopology::eTriangleList);

	auto const viewports = vk::Viewport()
        .setX(0.0f)
		.setY(0.0f)
		.setWidth(renderTarget->getResolution().width)
		.setHeight( renderTarget->getResolution().width)
		.setMinDepth( 0.0f)
		.setMaxDepth( 1.0f);

	auto const scissor = vk::Rect2D()
		.setOffset(vk::Offset2D(0,0))
		.setExtent(renderTarget->getResolution());

	auto const viewportStateInfo = vk::PipelineViewportStateCreateInfo()
		.setScissorCount(1)
		.setPScissors(&scissor)
		.setViewportCount(1)
		.setPViewports(&viewports);

	auto const rasterizationInfo = vk::PipelineRasterizationStateCreateInfo()
		.setCullMode(vk::CullModeFlagBits::eNone)
		.setDepthBiasClamp(0.0f)
		.setDepthBiasConstantFactor(0.0f)
		.setDepthBiasEnable(0)
		.setDepthBiasSlopeFactor(0.0f)
		.setDepthBiasEnable(0)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setLineWidth(0.0f)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setRasterizerDiscardEnable(0);

	auto const multisampleStateInfo = vk::PipelineMultisampleStateCreateInfo()
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);

	auto const noopStencilState = vk::StencilOpState()
		.setFailOp(vk::StencilOp::eKeep)
		.setPassOp(vk::StencilOp::eKeep)
		.setDepthFailOp(vk::StencilOp::eKeep)
		.setCompareOp(vk::CompareOp::eAlways);

	auto const depthStateInfo = vk::PipelineDepthStencilStateCreateInfo()
		.setDepthTestEnable(1)
		.setDepthWriteEnable(1)
		.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
		.setDepthBoundsTestEnable(0)
		.setStencilTestEnable(0)
		.setFront(noopStencilState)
		.setBack(noopStencilState)
		.setMaxDepthBounds(1.0f)
		.setMinDepthBounds(0.0f);

	std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates(renderTarget->getColourAttachmentNum());
	for (auto& a : colorBlendAttachmentStates) {
		a = vk::PipelineColorBlendAttachmentState()
			.setBlendEnable(0)
			.setSrcColorBlendFactor(vk::BlendFactor::eSrcColor)
			.setSrcColorBlendFactor(vk::BlendFactor::eOneMinusDstColor)
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setAlphaBlendOp(vk::BlendOp::eAdd);
	};

	auto const colorBlendState = vk::PipelineColorBlendStateCreateInfo()
		.setAttachmentCount(renderTarget->getColourAttachmentNum())
		.setPAttachments(colorBlendAttachmentStates.data());

	vk::DynamicState const dynamicState[2] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

	auto const dynamicStateInfo = vk::PipelineDynamicStateCreateInfo()
		.setDynamicStateCount(2)
		.setPDynamicStates(dynamicState);

	std::vector<vk::DescriptorSetLayoutBinding> layoutBinding(layouts.size());
	for (int i = 0; i < layouts.size(); i++) {
		layoutBinding[i] = vk::DescriptorSetLayoutBinding()
			.setBinding(layouts[i].binding)
			.setDescriptorType(layouts[i].type)
			.setDescriptorCount(1)
			.setStageFlags(layouts[i].stage);
	}

	auto const descriptorLayout = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(layoutBinding.size())
		.setPBindings(layoutBinding.data());

	std::vector<vk::DescriptorSetLayout> descLayouts(1);
	device->createDescriptorSetLayout(&descriptorLayout, nullptr, &descLayouts[0]);

	auto const layoutCreateInfo = vk::PipelineLayoutCreateInfo()
		.setSetLayoutCount(descLayouts.size())
		.setPSetLayouts(descLayouts.data());
	vk::PipelineLayout pipelineLayout;
	device->createPipelineLayout(&layoutCreateInfo, nullptr, &pipelineLayout);

	vk::PipelineCache pipelineCache;
	vk::PipelineCacheCreateInfo const pipelineCacheInfo;
	auto result = device->createPipelineCache(&pipelineCacheInfo, nullptr, &pipelineCache);
	assert(result == vk::Result::eSuccess);

	auto const graphicPipelineInfo = vk::GraphicsPipelineCreateInfo()
		.setStageCount(2)
		.setPStages(shaderStageCreateInfos)
		.setPVertexInputState(&vertexInputStateInfo)
		.setPInputAssemblyState(&vertexInputAssemblyStateInfo)
		.setPViewportState(&viewportStateInfo)
		.setPRasterizationState(&rasterizationInfo)
		.setPMultisampleState(&multisampleStateInfo)
		.setPDepthStencilState(&depthStateInfo)
		.setPColorBlendState(&colorBlendState)
		.setPDynamicState(&dynamicStateInfo)
		.setLayout(pipelineLayout)
		.setRenderPass(renderTarget->getRenderPass());

	auto pipline = device->createGraphicsPipeline(pipelineCache, graphicPipelineInfo);

	device->destroyShaderModule(vertRes.first);
	device->destroyShaderModule(fragRes.first);
	return new Shader(device, pipline, pipelineLayout, pipelineCache, descLayouts);
}

Shader::Shader(EnDevice * device, vk::Pipeline pipeline, vk::PipelineLayout pipelineLayout, vk::PipelineCache pipelineCache, vector<vk::DescriptorSetLayout> desSetLayout)
{
	_device = device;
	_pipeline = pipeline;
	_pipelineLayout = pipelineLayout;
	_pipelineCache = pipelineCache;
	_desSetLayout = desSetLayout;
}


Shader::~Shader()
{
	_device->destroyPipeline(_pipeline);
	_device->destroyPipelineCache(_pipelineCache);
	_device->destroyPipelineLayout(_pipelineLayout);
	for (auto& des : _desSetLayout) {
		_device->destroyDescriptorSetLayout(des);
	}
}
