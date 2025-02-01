#include "TriangleMesh.h"

TriangleMesh::TriangleMesh(const vk::PhysicalDevice& physicalDevice,
	const vk::Device& logicalDevice)
{
	std::vector<float> vertexData = {
		1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -0.05f, 0.0f,
		1.0f, 0.0f, 1.0f, 1.0f, 0.05f, 0.05f, 0.0f,
		1.0f, 0.0f, 1.0f, 1.0f, -0.05f, 0.05f, 0.0f
	};

	vkUtil::BufferInput inputChunk{};
	inputChunk.logicalDevice = logicalDevice;
	inputChunk.physicalDevice = physicalDevice;
	inputChunk.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	inputChunk.size = sizeof(float) * vertexData.size();
	
	vertexBufferData = vkUtil::create_buffer(inputChunk);

	void* memoryLocation = logicalDevice.mapMemory(vertexBufferData.bufferMemory, 0, inputChunk.size);
	memcpy(memoryLocation, vertexData.data(), inputChunk.size);
	logicalDevice.unmapMemory(vertexBufferData.bufferMemory);
}

vk::Buffer TriangleMesh::getVertexBuffer()
{
	return vertexBufferData.buffer;
}

void TriangleMesh::cleanup(const vk::Device& logicalDevice) const
{
	logicalDevice.destroyBuffer(vertexBufferData.buffer);
	logicalDevice.freeMemory(vertexBufferData.bufferMemory);

	delete this;
}

TriangleMesh::~TriangleMesh()
{
	
}