#pragma once

#include "Mesh.h"
#include <experimental/filesystem>

using namespace std::experimental::filesystem;

class ResourceManger
{
public:
	ResourceManger();
	~ResourceManger();

	Mesh* getAsset(path path);
};

