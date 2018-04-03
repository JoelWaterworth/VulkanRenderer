#include "mesh.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h> 

Mesh* Mesh::Create(Device* device, path p)
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
		const aiVector3D* pNor = &(mesh->mNormals[i]);
		const aiVector3D* pUV = &(mesh->mTextureCoords[0][i]);
		vertexBuffer.push_back(pPos->x);
		vertexBuffer.push_back(pPos->y);
		vertexBuffer.push_back(pPos->z);

		vertexBuffer.push_back(pNor->x);
		vertexBuffer.push_back(pNor->y);
		vertexBuffer.push_back(pNor->z);

		vertexBuffer.push_back(pUV->x);
		vertexBuffer.push_back(pUV->y);
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

Mesh::Mesh(Device* device, std::vector<float> vertexData, std::vector<unsigned int> indexData)
{
	_device = device;
	int32_t vertexBufferSize = vertexData.size() * sizeof(float);
	int32_t indexBufferSize = indexData.size() * sizeof(unsigned int);
	vertexBuffer = EnBuffer::Create(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBufferSize, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	indexBuffer = EnBuffer::Create(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBufferSize, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	this->indexBufferLen = indexData.size();
	
	void* vertexPtr = vertexBuffer->mapMemory(device);
	memcpy(vertexPtr, vertexData.data(), (size_t)vertexBufferSize);
	vertexBuffer->unMapMemory(device);
	void* indexPtr = indexBuffer->mapMemory(device);
	memcpy(indexPtr, indexData.data(), (size_t)indexBufferSize);
	indexBuffer->unMapMemory(device);
}

Mesh::~Mesh()
{
	delete indexBuffer;
	delete vertexBuffer;
}

void Mesh::draw(VkCommandBuffer commandBuffer) {
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer->buffer, &vertexOffset);

	vkCmdBindIndexBuffer(
		commandBuffer,
			indexBuffer->buffer,
			indexOffset,
			VK_INDEX_TYPE_UINT32);
	
	vkCmdDrawIndexed(
		commandBuffer,
			indexBufferLen,
			1,
			0,
			0,
			1);
}

void Mesh::destroy(Device * device)
{
	indexBuffer->destroy(device);
	vertexBuffer->destroy(device);
}

void Mesh::bindMemory(Device * device, VkDeviceMemory memory, uint64_t localOffset)
{
}

void Mesh::setBufferName(Device * device, const char * name)
{
	vertexBuffer->setObjectName(device, name);
}