#include "enBuffer.h"

EnBuffer::EnBuffer() {

}

EnBuffer* EnBuffer::Create(Device* device, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags flags)
{
	EnBuffer* buffer = new EnBuffer();
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vkCreateBuffer(device->handle(), &bufferInfo, nullptr, &buffer->buffer);
	vkGetBufferMemoryRequirements(device->handle(), buffer->buffer, &buffer->requirments);
	buffer->size = size;
	device->attachResource(buffer, flags);
	return buffer;
}


void EnBuffer::bindMemory(Device* device, VkDeviceMemory memory, uint64_t localOffset) {
	vkBindBufferMemory(device->handle(), buffer, memory, _offset + localOffset);
}

void EnBuffer::destroy(Device* device) {
	vkDestroyBuffer(device->handle(), buffer, nullptr);
}

void * EnBuffer::mapMemory(Device* device)
{
	void * ptr = nullptr;
	vkMapMemory(device->handle(),memory, _offset, size, 0, &ptr);
	return ptr;
}

void EnBuffer::unMapMemory(Device* device)
{
	vkUnmapMemory(device->handle(), memory);
}

void EnBuffer::setObjectName(Device * device, const char * name)
{
	device->setObjectName((uint64_t)buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
}
