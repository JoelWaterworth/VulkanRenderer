#include "Resource.h"


void Resource::bindMemory(EnDevice* device, vk::DeviceMemory memory, uint64_t localOffset)
{
}

void Resource::destroy(EnDevice* device)
{
}

bool Resource::postAllocation(EnDevice* device)
{
	return false;
}
