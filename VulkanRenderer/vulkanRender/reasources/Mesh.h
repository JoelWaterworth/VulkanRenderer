#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include "../EnDevice.h"
#include "Resource.h"
#include <vector>
#include <glm/vec3.hpp>
#include <experimental/filesystem>

using namespace std::experimental::filesystem;
/*
enum class Components : unsigned { BiTangent = 0x4, TexCoord = 0x2, Normal = 0x1 };

Components operator |(Components lhs, Components rhs)
{
	return static_cast<Components> (
		static_cast<unsigned>(lhs) |
		static_cast<unsigned>(rhs)
		);
}
*/
class Mesh : public Resource
{
public:
	static Mesh* Create(EnDevice* device, path p);
	Mesh(EnDevice* device, std::vector<float> vertexData, std::vector<unsigned int> indexData);
	~Mesh();
private:
	EnDevice* device = nullptr;
	vk::DeviceMemory indexMemory;
	vk::DeviceMemory vertexMemory;
	vk::Buffer indexBuffer;
	vk::Buffer vertexBuffer;
	uint32_t indexBufferLen;
	uint64_t indexOffset;
	uint64_t vertexOffset;
};

