#include "EnBuffer.h"

EnBuffer::EnBuffer() {

}

EnBuffer* EnBuffer::Create(EnDevice* device, vk::BufferUsageFlags usage, vk::DeviceSize size, vk::MemoryPropertyFlags flags)
{
	EnBuffer* buffer = new EnBuffer();
	auto const bufferInfo = vk::BufferCreateInfo()
		.setSize(size)
		.setUsage(usage)
		.setSharingMode(vk::SharingMode::eExclusive);
	buffer->buffer = device->createBuffer(bufferInfo);
	buffer->requirments = device->getBufferMemoryRequirements(buffer->buffer);
	buffer->size = size;
	device->attachResource(buffer, flags);
	return buffer;
}


void EnBuffer::bindMemory(EnDevice* device, vk::DeviceMemory memory, uint64_t localOffset) {
	device->bindBufferMemory(buffer, memory, _offset + localOffset);
}

void EnBuffer::destroy(EnDevice* device) {
	device->destroyBuffer(buffer);
}

void * EnBuffer::mapMemory(EnDevice* device)
{
	return device->mapMemory(memory, _offset, size);
}

void EnBuffer::unMapMemory(EnDevice* device)
{
	device->unmapMemory(memory);
}

void EnBuffer::setObjectName(EnDevice * device, const char * name)
{
	VkBuffer b = *reinterpret_cast<VkBuffer*>(&buffer);
	device->setObjectName((uint64_t)b, vk::DebugReportObjectTypeEXT::eBuffer, name);
}
