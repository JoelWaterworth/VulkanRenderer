#pragma once
#include <vulkan/vulkan.h>
#include "../device.h"
#include "enBuffer.h"
#include "resource.h"
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
	static Mesh Create(Device* device, path p);
	Mesh(Device* device, std::vector<float> vertexData, std::vector<unsigned int> indexData);
	Mesh() :
		_device(nullptr),
		_indexBuffer(nullptr),
		_vertexBuffer(nullptr),
		_indexBufferLen(0),
		_indexOffset(0),
		_vertexOffset(0)
	{};
	void bind(VkCommandBuffer commandBuffer);
	void draw(VkCommandBuffer commandBuffer);
	virtual void destroy(Device* device);
	virtual void bindMemory(Device* device, VkDeviceMemory memory, uint64_t localOffset);
	void setBufferName(Device* device, const char* name);
private:
	Device * _device;
	EnBuffer* _indexBuffer;
	EnBuffer* _vertexBuffer;
	uint32_t _indexBufferLen;
	uint64_t _indexOffset = 0;
	uint64_t _vertexOffset = 0;
};

