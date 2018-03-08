#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include <experimental/filesystem>

#include "EnDevice.h"
#include "RenderTarget.h"

using namespace std::experimental::filesystem;
using namespace std;

struct ShaderLayout {
	vk::DescriptorType type;
	vk::ShaderStageFlags stage;
	uint32_t binding;
	uint32_t set;

	ShaderLayout() :
		type(vk::DescriptorType::eUniformBuffer),
		stage(vk::ShaderStageFlagBits::eFragment),
		binding(0),
		set(0) {};

	ShaderLayout(
		vk::DescriptorType Type ,
		vk::ShaderStageFlags Stage,
		uint32_t Binding,
		uint32_t Set) :
		type(Type),
		stage(Stage),
		binding(Binding),
		set(Set) {};

};

class Shader
{
public:
	static Shader* Create(EnDevice* device, RenderTarget* renderTarget, path vertPath, path fragPath, vector<ShaderLayout> layouts);
	Shader();
	~Shader();

	inline vk::Pipeline GetPipeline() const { return _pipeline; }
	inline vk::PipelineLayout GetPipelineLayout() const { return _pipelineLayout; }
	inline vector<vk::DescriptorType> getTypes() const { return _types; }
	inline vector<vk::DescriptorSetLayout> getDesSetLayout() const { return _desSetLayout; }

private:
	EnDevice* _device;
	vk::Pipeline _pipeline;
	vk::PipelineLayout _pipelineLayout;
	vk::PipelineCache _pipelineCache;
	vector<vk::DescriptorSetLayout> _desSetLayout;
	vector<vk::DescriptorType> _types;

	static pair<char *, size_t> compile(path shader);
	static char* readSpv(const char *filename, size_t *psize);
	static vk::ShaderModule createShaderModule(EnDevice* device, const void *code, size_t size);
	static pair<vk::ShaderModule, vk::PipelineShaderStageCreateInfo> createPipelineStageInfo(EnDevice* device, path shader, vk::ShaderStageFlagBits stage);

};

