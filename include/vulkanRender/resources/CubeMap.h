#pragma once
#include "resource.h"
#include <vulkan/vulkan.h>
#include "../uniformInterface.h"
#include <experimental/filesystem>

using namespace std::experimental::filesystem;

class CubeMap : public Resource, public UniformInterface
{
public:
	CubeMap();
	~CubeMap();
};

