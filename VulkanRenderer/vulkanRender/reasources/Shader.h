#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include <experimental/filesystem>

#include "../EnDevice.h"
#include "../RenderTarget.h"

using namespace std::experimental::filesystem;
using namespace std;

struct ShaderLayout {
	vk::DescriptorType type;
	vk::ShaderStageFlags stage;
	uint32_t binding;
	uint32_t set;
};

class Shader
{
public:
	static Shader* Create(EnDevice* device, RenderTarget* renderTarget, path vertPath, path fragPath, vector<ShaderLayout> layouts);
	Shader(EnDevice * device, vk::Pipeline pipeline, vk::PipelineLayout pipelineLayout, vk::PipelineCache pipelineCache, vector<vk::DescriptorSetLayout> desSetLayout);
	~Shader();

	inline vk::Pipeline GetPipeline() const { return _pipeline; }
	inline vk::PipelineLayout GetPipelineLayout() const { return _pipelineLayout; }

private:
	EnDevice* _device;
	vk::Pipeline _pipeline;
	vk::PipelineLayout _pipelineLayout;
	vk::PipelineCache _pipelineCache;
	vector<vk::DescriptorSetLayout> _desSetLayout;
	static pair<char *, size_t> compile(path shader);
	static char* readSpv(const char *filename, size_t *psize);
	static vk::ShaderModule createShaderModule(EnDevice* device, const void *code, size_t size);
	static pair<vk::ShaderModule, vk::PipelineShaderStageCreateInfo> createPipelineStageInfo(EnDevice* device, path shader, vk::ShaderStageFlagBits stage);
};

