#include "shader.h"
#include "util.h"
#include <iostream>
#include <stdio.h>
#include <fstream>

void Shader::destroy(Device * device)
{
	vkDestroyPipeline(device->handle(), _pipeline, nullptr);
	if (_pipelineCache != VK_NULL_HANDLE) {
		vkDestroyPipelineCache(device->handle(), _pipelineCache, nullptr);
	}
	vkDestroyPipelineLayout(device->handle(), _pipelineLayout, nullptr);
	for (auto& des : _desSetLayouts) {
		vkDestroyDescriptorSetLayout(device->handle(), des, nullptr);
	}
}

std::pair<char *, size_t> Shader::compile(path shader) {
	path spv_path = shader;
	std::string filename = shader.filename().string();
	filename.append(".spv");
	spv_path.replace_filename(filename);
	std::string command = "glslangValidator -V ";
	command.append(shader.string());
	command.append(" -o ");
	command.append(spv_path.string());
	system(command.data());
	FILE *fp = fopen(spv_path.string().data(), "rb");
	if (!fp) {
	  printf("unable to compile shader");
		return std::pair<char *, size_t>();
	}

	fseek(fp, 0L, SEEK_END);
	long int size = ftell(fp);
  assert(size > 0);
	fseek(fp, 0L, SEEK_SET);

	void *shader_code = malloc(size);
	size_t retval = fread(shader_code, size, 1, fp);
	assert(retval == 1);

	fclose(fp);

	return std::make_pair((char *)shader_code, size);
}

VkShaderModule Shader::createShaderModule(Device* device, const void * code, size_t size){
	VkShaderModuleCreateInfo moduleCreateInfo = {};
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.codeSize = size;
	moduleCreateInfo.pCode = (uint32_t const *)code;

	VkShaderModule module;
	auto result = vkCreateShaderModule(device->handle(), &moduleCreateInfo, nullptr, &module);
	assert(result == VK_SUCCESS);

	return module;
}

pair<VkShaderModule, VkPipelineShaderStageCreateInfo> Shader::createPipelineStageInfo(Device* device, path shader, VkShaderStageFlagBits stage){
	std::pair<char *, size_t> s = compile(shader);
	VkShaderModule Module = Shader::createShaderModule(device, s.first, s.second);
	VkPipelineShaderStageCreateInfo pipelineStageInfo = {};
	pipelineStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineStageInfo.module = Module;
	pipelineStageInfo.pName = "main";
	pipelineStageInfo.stage = stage;
	return make_pair(Module, pipelineStageInfo);
}

Shader Shader::Create(
  Device * device,
  RenderTarget* renderTarget,
  path vertPath, path fragPath,
  vector<ShaderLayout> layouts,
  std::vector<VkPushConstantRange> consts,
  bool bIsDisableVertexDescriptor)
{
	pair<VkShaderModule, VkPipelineShaderStageCreateInfo> vertRes =
		Shader::createPipelineStageInfo(device, vertPath, VK_SHADER_STAGE_VERTEX_BIT);
	pair<VkShaderModule, VkPipelineShaderStageCreateInfo> fragRes =
		Shader::createPipelineStageInfo(device, fragPath, VK_SHADER_STAGE_FRAGMENT_BIT);

	VkPipelineShaderStageCreateInfo shaderStageCreateInfos[] = {
		vertRes.second, fragRes.second
	};

	VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
	vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	if (!bIsDisableVertexDescriptor) {
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
		
		vertexInputStateInfo.vertexAttributeDescriptionCount = 3;
		vertexInputStateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions;
		vertexInputStateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions;
	}
	else {
		vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
		vertexInputStateInfo.pVertexAttributeDescriptions = nullptr;
		vertexInputStateInfo.vertexBindingDescriptionCount = 0;
		vertexInputStateInfo.pVertexBindingDescriptions = nullptr;
	}

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
		a.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		a.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
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

	uint8_t descriptorCount = 0;
	std::vector<std::vector<ShaderLayout>> layout2D;
	for (int i = 0; i < layouts.size(); i++) {
		if (layout2D.size() <= layouts[i].set) {
			layout2D.push_back(std::vector<ShaderLayout>());
		}
		layout2D[layouts[i].set].push_back(layouts[i]);
		descriptorCount = (descriptorCount <= layouts[i].set) ? (layouts[i].set + 1) : descriptorCount;
	}
	std::vector<std::vector<VkDescriptorSetLayoutBinding>> layoutBindings;

	for (int i = 0; i < layouts.size(); i++) {
		uint8_t s = layouts[i].set;
		if (layoutBindings.size() <= s) {
			layoutBindings.push_back(std::vector<VkDescriptorSetLayoutBinding>());
		}
		VkDescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.binding = layouts[i].binding;
		layoutBinding.descriptorType = layouts[i].type;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = layouts[i].stage;
		layoutBindings[s].push_back(layoutBinding);

		descriptorCount = (descriptorCount < (layouts[i].set + 1)) ? (layouts[i].set + 1) : descriptorCount;
	}

	std::vector<VkDescriptorSetLayoutCreateInfo> descriptorLayouts(layoutBindings.size());
	for (int i = 0; i < layoutBindings.size(); i++) {
		descriptorLayouts[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayouts[i].bindingCount = layoutBindings[i].size();
		descriptorLayouts[i].pBindings = layoutBindings[i].data();
	}
	printf("pre vkCreateDescriptorSetLayout\n");
	std::vector<VkDescriptorSetLayout> descLayouts(layoutBindings.size());
	for (int i = 0; i < layoutBindings.size(); i++) {
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device->handle(), &descriptorLayouts[i], nullptr, &descLayouts[i]));
	}

	VkPipelineLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCreateInfo.setLayoutCount = descriptorCount;
	layoutCreateInfo.pSetLayouts = descLayouts.data();
	layoutCreateInfo.pushConstantRangeCount = consts.size();
	layoutCreateInfo.pPushConstantRanges = consts.data();

	printf("pre vkCreatePipelineLayout\n");
	VkPipelineLayout pipelineLayout;
	VK_CHECK_RESULT(vkCreatePipelineLayout(device->handle(), &layoutCreateInfo, nullptr, &pipelineLayout));

	VkPipelineCache pipelineCache = VK_NULL_HANDLE;

	VkPipelineCacheCreateInfo pipelineCacheInfo = {};
	pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	printf("pre vkCreatePipelineCache\n");
	VK_CHECK_RESULT(vkCreatePipelineCache(device->handle(), &pipelineCacheInfo, nullptr, &pipelineCache));

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

	printf("pre create Pipeline\n");
	VkPipeline pipline;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device->handle(), pipelineCache, 1, &graphicPipelineInfo, nullptr, &pipline));

	vkDestroyShaderModule(device->handle(), vertRes.first, nullptr);
	vkDestroyShaderModule(device->handle(), fragRes.first, nullptr);
	auto shader = Shader();
	shader._pipeline = pipline;
	shader._pipelineLayout = pipelineLayout;
	shader._pipelineCache = pipelineCache;
	shader._desSetLayouts = descLayouts;
	shader._shaderLayout = layout2D;
	shader._descriptorCount = descriptorCount;
	return shader;
}

Shader::Shader()
{}
