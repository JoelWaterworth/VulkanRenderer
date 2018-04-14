#pragma once

#include <vulkan/vulkan.h>
#include <experimental/filesystem>

#include "device.h"
#include "renderTarget.h"

using namespace std::experimental::filesystem;
using namespace std;

struct ShaderLayout {
	VkDescriptorType type;
	VkShaderStageFlags stage;
	uint32_t binding;
	uint32_t set;

	ShaderLayout() :
		type(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		stage(VK_SHADER_STAGE_FRAGMENT_BIT),
		binding(0),
		set(0) {};

	ShaderLayout(
		VkDescriptorType Type ,
		VkShaderStageFlags Stage,
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
	static Shader* Create(Device* device, RenderTarget* renderTarget, path vertPath, path fragPath, vector<ShaderLayout> layouts, std::vector<VkPushConstantRange> consts = std::vector<VkPushConstantRange>(), bool bIsDisableVertexDescriptor = false);
	Shader();
	~Shader();

	vector<VkDescriptorSetLayout> _desSetLayouts;

	inline uint8_t getDescriptorCount() const { return _descriptorCount; }
	inline VkPipeline GetPipeline() const { return _pipeline; }
	inline VkPipelineLayout GetPipelineLayout() const { return _pipelineLayout; }
	inline vector<VkDescriptorSetLayout> getDesSetLayout() const { return _desSetLayouts; }
	inline vector<vector<ShaderLayout>> getShaderLayout() const { return _shaderLayout; }

private:
	uint8_t _descriptorCount;
	Device* _device;
	VkPipeline _pipeline = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
	VkPipelineCache _pipelineCache = VK_NULL_HANDLE;
	vector<vector<ShaderLayout>> _shaderLayout;

	static pair<char *, size_t> compile(path shader);
	static VkShaderModule createShaderModule(Device* device, const void *code, size_t size);
	static pair<VkShaderModule, VkPipelineShaderStageCreateInfo> createPipelineStageInfo(Device* device, path shader, VkShaderStageFlagBits stage);

};

