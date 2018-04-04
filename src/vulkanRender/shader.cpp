#include "shader.h"
#include "util.h"
#include <iostream>
#include <stdio.h>
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
	FILE *fp = fopen(spv_path.string().data(), "rb");
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

VkShaderModule Shader::createShaderModule(Device* device, const void * code, size_t size)
{
	VkShaderModuleCreateInfo moduleCreateInfo = {};
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.codeSize = size;
	moduleCreateInfo.pCode = (uint32_t const *)code;

	VkShaderModule module;
	auto result = vkCreateShaderModule(device->handle(), &moduleCreateInfo, nullptr, &module);
	assert(result == VK_SUCCESS);

	return module;
}

pair<VkShaderModule, VkPipelineShaderStageCreateInfo> Shader::createPipelineStageInfo(Device* device, path shader, VkShaderStageFlagBits stage)
{
	std::pair<char *, size_t> s = compile(shader);
	VkShaderModule Module = Shader::createShaderModule(device, s.first, s.second);
	VkPipelineShaderStageCreateInfo pipelineStageInfo = {};
	pipelineStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineStageInfo.module = Module;
	pipelineStageInfo.pName = "main";
	pipelineStageInfo.stage = stage;
	return make_pair(Module, pipelineStageInfo);
}

Shader* Shader::Create(Device * device, RenderTarget* renderTarget, path vertPath, path fragPath, vector<ShaderLayout> layouts)
{
	pair<VkShaderModule, VkPipelineShaderStageCreateInfo> vertRes =
		Shader::createPipelineStageInfo(device, vertPath, VK_SHADER_STAGE_VERTEX_BIT);
	pair<VkShaderModule, VkPipelineShaderStageCreateInfo> fragRes =
		Shader::createPipelineStageInfo(device, fragPath, VK_SHADER_STAGE_FRAGMENT_BIT);

	VkPipelineShaderStageCreateInfo shaderStageCreateInfos[] = {
		vertRes.second, fragRes.second
	};

	VkVertexInputBindingDescription vertexInputBindingDescriptions[1] = {};
	vertexInputBindingDescriptions[0].binding = 0;
	vertexInputBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexInputBindingDescriptions[0].stride = sizeof(float) * 8;

	VkVertexInputAttributeDescription vertexInputAttributeDescriptions[3] = {};
	vertexInputAttributeDescriptions[0].location = 0;
	vertexInputAttributeDescriptions[0].binding = 0;
	vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescriptions[0].offset = 0;

	vertexInputAttributeDescriptions[1].location = 1;
	vertexInputAttributeDescriptions[1].binding = 0;
	vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescriptions[1].offset = sizeof(float) * 3;

	vertexInputAttributeDescriptions[2].location = 2;
	vertexInputAttributeDescriptions[2].binding = 0;
	vertexInputAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttributeDescriptions[2].offset = sizeof(float) * 6;

	VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
	vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateInfo.vertexAttributeDescriptionCount = 3;
	vertexInputStateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions;
	vertexInputStateInfo.vertexBindingDescriptionCount = 1;
	vertexInputStateInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions;

	VkPipelineInputAssemblyStateCreateInfo vertexInputAssemblyStateInfo = {};
	vertexInputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	vertexInputAssemblyStateInfo.primitiveRestartEnable = 0;
	vertexInputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkViewport viewports = {};
	viewports.x = 0.0f;
	viewports.y = 0.0f;
	viewports.width = renderTarget->getResolution().width;
	viewports.height = renderTarget->getResolution().height;
	viewports.minDepth = 0.0f;
	viewports.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.extent = renderTarget->getResolution();

	VkPipelineViewportStateCreateInfo viewportStateInfo = {};
	viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateInfo.scissorCount = 1;
	viewportStateInfo.pScissors = &scissor;
	viewportStateInfo.viewportCount = 1;
	viewportStateInfo.pViewports = &viewports;

	VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
	rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
	rasterizationInfo.depthBiasEnable = 0;
	rasterizationInfo.depthBiasClamp = 0.0f;
	rasterizationInfo.depthBiasConstantFactor = 0.0f;
	rasterizationInfo.depthBiasConstantFactor = 0.0f;
	rasterizationInfo.depthBiasSlopeFactor = 0.0f;
	rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationInfo.lineWidth = 1.0f;
	rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationInfo.rasterizerDiscardEnable = 0;

	VkPipelineMultisampleStateCreateInfo multisampleStateInfo = {};
	multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkStencilOpState noopStencilState = {};
	noopStencilState.failOp = VK_STENCIL_OP_KEEP;
	noopStencilState.passOp = VK_STENCIL_OP_KEEP;
	noopStencilState.depthFailOp = VK_STENCIL_OP_KEEP;
	noopStencilState.compareOp = VK_COMPARE_OP_ALWAYS;

	VkPipelineDepthStencilStateCreateInfo depthStateInfo = {};
	depthStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStateInfo.depthTestEnable = 1;
	depthStateInfo.depthWriteEnable = 1;
	depthStateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStateInfo.depthBoundsTestEnable = 0;
	depthStateInfo.stencilTestEnable = 0;
	depthStateInfo.front = noopStencilState;
	depthStateInfo.back = noopStencilState;
	depthStateInfo.maxDepthBounds = 1.0f;
	depthStateInfo.minDepthBounds = 0.0f;

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates(renderTarget->getColourAttachmentNum());
	for (auto& a : colorBlendAttachmentStates) {
		a.blendEnable = 0;
		a.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
		a.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		a.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		a.colorBlendOp = VK_BLEND_OP_ADD;
		a.alphaBlendOp = VK_BLEND_OP_ADD;
	};

	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = 0;
	colorBlendState.logicOp = VK_LOGIC_OP_CLEAR;
	colorBlendState.attachmentCount = colorBlendAttachmentStates.size();
	colorBlendState.pAttachments = colorBlendAttachmentStates.data();

	VkDynamicState const dynamicState[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = 2;
	dynamicStateInfo.pDynamicStates = dynamicState;

	std::vector<VkDescriptorType> types(layouts.size());
	std::vector<VkDescriptorSetLayoutBinding> layoutBinding(layouts.size());
	uint8_t descriptorCount = 0;
	for (int i = 0; i < layouts.size(); i++) {
		layoutBinding[i].binding = layouts[i].binding;
		layoutBinding[i].descriptorType = layouts[i].type;
		layoutBinding[i].descriptorCount = 1;
		layoutBinding[i].stageFlags = layouts[i].stage;
		types[i] = layouts[i].type;

		descriptorCount = (descriptorCount < (layouts[i].set + 1)) ? (layouts[i].set + 1) : descriptorCount;
	}

	VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.bindingCount = layoutBinding.size();
	descriptorLayout.pBindings = layoutBinding.data();

	VkDescriptorSetLayout descLayouts;
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device->handle(), &descriptorLayout, nullptr, &descLayouts));

	VkPipelineLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCreateInfo.setLayoutCount = 1;
	layoutCreateInfo.pSetLayouts = &descLayouts;

	VkPipelineLayout pipelineLayout;
	VK_CHECK_RESULT(vkCreatePipelineLayout(device->handle(), &layoutCreateInfo, nullptr, &pipelineLayout));

	VkPipelineCache pipelineCache;
	VkPipelineCacheCreateInfo pipelineCacheInfo;
	pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCacheInfo.flags = 0;
	pipelineCacheInfo.initialDataSize = 0;
	auto result = vkCreatePipelineCache(device->handle(), &pipelineCacheInfo, nullptr, &pipelineCache);
	assert(result == VK_SUCCESS);

	VkGraphicsPipelineCreateInfo graphicPipelineInfo = {};
	graphicPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicPipelineInfo.stageCount = 2;
	graphicPipelineInfo.pStages = shaderStageCreateInfos;
	graphicPipelineInfo.pVertexInputState = &vertexInputStateInfo;
	graphicPipelineInfo.pInputAssemblyState = &vertexInputAssemblyStateInfo;
	graphicPipelineInfo.pViewportState = &viewportStateInfo;
	graphicPipelineInfo.pRasterizationState = &rasterizationInfo;
	graphicPipelineInfo.pMultisampleState = &multisampleStateInfo;
	graphicPipelineInfo.pDepthStencilState = &depthStateInfo;
	graphicPipelineInfo.pColorBlendState = &colorBlendState;
	graphicPipelineInfo.pDynamicState = &dynamicStateInfo;
	graphicPipelineInfo.layout = pipelineLayout;
	graphicPipelineInfo.renderPass = renderTarget->getRenderPass();

	VkPipeline pipline;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device->handle(), pipelineCache, 1, &graphicPipelineInfo, nullptr, &pipline));

	vkDestroyShaderModule(device->handle(), vertRes.first, nullptr);
	vkDestroyShaderModule(device->handle(), fragRes.first, nullptr);
	auto shader = new Shader();
	shader->_device = device;
	shader->_pipeline = pipline;
	shader->_pipelineLayout = pipelineLayout;
	shader->_pipelineCache = pipelineCache;
	shader->_desSetLayout = descLayouts;
	shader->_types = types;
	shader->_descriptorCount = descriptorCount;
	return shader;
}

Shader::Shader()
{}


Shader::~Shader()
{
	vkDestroyPipeline(_device->handle(), _pipeline, nullptr);
	vkDestroyPipelineCache(_device->handle(), _pipelineCache, nullptr);
	vkDestroyPipelineLayout(_device->handle(), _pipelineLayout, nullptr);
	//for (auto& des : _desSetLayout) {
		vkDestroyDescriptorSetLayout(_device->handle(), _desSetLayout, nullptr);
	//}
}
