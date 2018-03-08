#pragma once

#include "Mesh.h"
#include "Resource.h"
#include <experimental/filesystem>
using namespace std;
using namespace std::experimental::filesystem;

struct Allocation {
	vk::DeviceMemory allocation;
	vector<Resource*> resources;
};

class ResourceManger
{
public:
	ResourceManger(EnDevice* device);
	~ResourceManger();

	void allocate(const vector<Resource*>& resources, vk::MemoryPropertyFlags memoryFlags);

	Mesh* getAsset(path path);
private:
	vector<Allocation> allocations;
	EnDevice* _device = nullptr;
};

