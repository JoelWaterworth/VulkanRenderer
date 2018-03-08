#include "Mesh.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h> 

Mesh* Mesh::Create(EnDevice* device, path p)
{
	Assimp::Importer importer;
	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll 
	// propably to request more postprocessing than we do in this example.
	const aiScene* scene = importer.ReadFile(p.string(),
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	// If the import failed, report it
	if (!scene)
	{
		std::cout << "Error loading file: (assimp:) " << importer.GetErrorString();
		return nullptr;
	}

	if (!scene->HasMeshes()) {
		return nullptr;
	}
	std::vector<float> vertexBuffer;
	aiMesh* mesh = scene->mMeshes[0];
	for (int i = 0; i < mesh->mNumVertices; i++) {
		const aiVector3D* pPos = &(mesh->mVertices[i]);
		const aiVector3D* pNormal = &(mesh->mNormals[i]);
		vertexBuffer.push_back(pPos->x);
		vertexBuffer.push_back(pPos->y);
		vertexBuffer.push_back(pPos->z);
	}

	std::vector<unsigned int> indexBuffer;
	for (int i = 0; i < mesh->mNumFaces; i++) {
		const aiFace& face = mesh->mFaces[i];
		if (face.mNumIndices != 3)
			continue;
		indexBuffer.push_back(face.mIndices[0]);
		indexBuffer.push_back(face.mIndices[1]);
		indexBuffer.push_back(face.mIndices[2]);
	}
	return new Mesh(device, vertexBuffer, indexBuffer);
}

Mesh::Mesh(EnDevice* device, std::vector<float> vertexData, std::vector<unsigned int> indexData)
{
	this->_device = device;
	int32_t vertexBufferSize = vertexData.size() * sizeof(float);
	int32_t indexBufferSize = indexData.size() * sizeof(unsigned int);

	std::pair<vk::Buffer, vk::DeviceMemory> vertex = device->allocateBuffer(vertexBufferSize, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	std::pair<vk::Buffer, vk::DeviceMemory> index = device->allocateBuffer(indexBufferSize, vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	this->vertexBuffer = vertex.first;
	this->vertexMemory = vertex.second;
	this->indexBuffer = index.first;
	this->indexMemory = index.second;
	this->indexBufferLen = indexData.size();
	void* vertexPtr = device->mapMemory(vertexMemory, 0, vertexBufferSize);
	memcpy(vertexPtr, vertexData.data(), (size_t)vertexBufferSize);
	device->unmapMemory(vertex.second);
	void* indexPtr = device->mapMemory(indexMemory, 0, indexBufferSize);
	memcpy(indexPtr, indexData.data(), (size_t)indexBufferSize);
	device->unmapMemory(index.second);
}

Mesh::~Mesh()
{
	_device->destroyBuffer(indexBuffer);
	_device->destroyBuffer(vertexBuffer);
	_device->freeMemory(indexMemory);
	_device->freeMemory(vertexMemory);
}

void Mesh::draw(vk::CommandBuffer commandBuffer) {
	commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer, &vertexOffset);

	commandBuffer.bindIndexBuffer(
			indexBuffer,
			indexOffset,
			vk::IndexType::eUint32);
	
	commandBuffer.drawIndexed(
			indexBufferLen,
			1,
			0,
			0,
			1);
}
