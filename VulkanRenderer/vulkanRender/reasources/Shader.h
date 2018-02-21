#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include <experimental/filesystem>

#include "../EnDevice.h"
#include "../RenderTarget.h"

using namespace std::experimental::filesystem;
using namespace std;

class Shader
{
public:
	static Shader* Create(EnDevice* device, RenderTarget* renderTarget, path vertPath, path fragPath);
	Shader();
	~Shader();
private:
	EnDevice* device;

	static pair<const void*, size_t> compile(path shader);

	static vk::ShaderModule createShaderModule(EnDevice* device, const void *code, size_t size);
	static pair<vk::ShaderModule, vk::PipelineShaderStageCreateInfo> createPipelineStageInfo(EnDevice* device, path shader, vk::ShaderStageFlagBits stage);
};

