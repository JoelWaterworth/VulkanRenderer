#include "Shader.h"
#include <iostream>
#include <fstream>

std::pair<const void*, size_t> Shader::compile(path shader) {
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
		return std::pair<const void*, size_t>();
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
	std::pair<const void*, unsigned int> res = Shader::compile(shader);
	vk::ShaderModule Module = Shader::createShaderModule(device, res.first, res.second);
	vk::PipelineShaderStageCreateInfo pipelineStageInfo = vk::PipelineShaderStageCreateInfo()
		.setModule(Module)
		.setPName("main")
		.setStage(stage);
	return make_pair(Module, pipelineStageInfo);
}

Shader* Shader::Create(EnDevice * device, RenderTarget* renderTarget, path vertPath, path fragPath)
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
	return nullptr;

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

	auto const multisample_state_info = vk::PipelineMultisampleStateCreateInfo()
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

        let color_blend_attachment_states = if deferred { vec![vk::PipelineColorBlendAttachmentState {
                blend_enable: 0,
                src_color_blend_factor: vk::BlendFactor::SrcColor,
                dst_color_blend_factor:
                vk::BlendFactor::OneMinusDstColor,
                color_blend_op: vk::BlendOp::Add,
                src_alpha_blend_factor: vk::BlendFactor::Zero,
                dst_alpha_blend_factor: vk::BlendFactor::Zero,
                alpha_blend_op: vk::BlendOp::Add,
                color_write_mask: vk::ColorComponentFlags::all(),
            },
            vk::PipelineColorBlendAttachmentState {
                blend_enable: 0,
                src_color_blend_factor: vk::BlendFactor::SrcColor,
                dst_color_blend_factor:
                vk::BlendFactor::OneMinusDstColor,
                color_blend_op: vk::BlendOp::Add,
                src_alpha_blend_factor: vk::BlendFactor::Zero,
                dst_alpha_blend_factor: vk::BlendFactor::Zero,
                alpha_blend_op: vk::BlendOp::Add,
                color_write_mask: vk::ColorComponentFlags::all(),
            },
            vk::PipelineColorBlendAttachmentState {
                blend_enable: 0,
                src_color_blend_factor: vk::BlendFactor::SrcColor,
                dst_color_blend_factor:
                vk::BlendFactor::OneMinusDstColor,
                color_blend_op: vk::BlendOp::Add,
                src_alpha_blend_factor: vk::BlendFactor::Zero,
                dst_alpha_blend_factor: vk::BlendFactor::Zero,
                alpha_blend_op: vk::BlendOp::Add,
                color_write_mask: vk::ColorComponentFlags::all(),
            }

        ]} else {
            vec![vk::PipelineColorBlendAttachmentState {
                blend_enable: 0,
                src_color_blend_factor: vk::BlendFactor::SrcColor,
                dst_color_blend_factor:
                vk::BlendFactor::OneMinusDstColor,
                color_blend_op: vk::BlendOp::Add,
                src_alpha_blend_factor: vk::BlendFactor::Zero,
                dst_alpha_blend_factor: vk::BlendFactor::Zero,
                alpha_blend_op: vk::BlendOp::Add,
                color_write_mask: vk::ColorComponentFlags::all(),
            }]
        };

        let color_blend_state = vk::PipelineColorBlendStateCreateInfo {
            s_type: vk::StructureType::PipelineColorBlendStateCreateInfo,
            p_next: ptr::null(),
            flags: Default::default(),
            logic_op_enable: 0,
            logic_op: vk::LogicOp::Clear,
            attachment_count: color_blend_attachment_states.len() as u32,
            p_attachments: color_blend_attachment_states.as_ptr(),
            blend_constants: [0.0, 0.0, 0.0, 0.0],
        };

        let dynamic_state = [vk::DynamicState::Viewport, vk::DynamicState::Scissor];
        let dynamic_state_info = vk::PipelineDynamicStateCreateInfo {
            s_type: vk::StructureType::PipelineDynamicStateCreateInfo,
            p_next: ptr::null(),
            flags: Default::default(),
            dynamic_state_count: dynamic_state.len() as u32,
            p_dynamic_states: dynamic_state.as_ptr(),
        };

        let layout_create_info = vk::PipelineLayoutCreateInfo {
            s_type: vk::StructureType::PipelineLayoutCreateInfo,
            p_next: ptr::null(),
            flags: Default::default(),
            set_layout_count: descriptor_set_layout.len() as u32,
            p_set_layouts: descriptor_set_layout.as_ptr(),
            push_constant_range_count: 0,
            p_push_constant_ranges: ptr::null(),
        };

        let pipeline_layout =
            device.create_pipeline_layout(&layout_create_info, None).unwrap();

        let graphic_pipeline_info = vk::GraphicsPipelineCreateInfo {
            s_type: vk::StructureType::GraphicsPipelineCreateInfo,
            p_next: ptr::null(),
            flags: vk::PipelineCreateFlags::empty(),
            stage_count: shader_stage_create_infos.len() as u32,
            p_stages: shader_stage_create_infos.as_ptr(),
            p_vertex_input_state: &vertex_input_state_info,
            p_input_assembly_state: &vertex_input_assembly_state_info,
            p_tessellation_state: ptr::null(),
            p_viewport_state: &viewport_state_info,
            p_rasterization_state: &rasterization_info,
            p_multisample_state: &multisample_state_info,
            p_depth_stencil_state: &depth_state_info,
            p_color_blend_state: &color_blend_state,
            p_dynamic_state: &dynamic_state_info,
            layout: pipeline_layout,
            render_pass: render_pass.clone(),
            subpass: 0,
            base_pipeline_handle: vk::Pipeline::null(),
            base_pipeline_index: 0,
}

Shader::Shader()
{
}


Shader::~Shader()
{
}
