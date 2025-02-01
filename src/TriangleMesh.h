#pragma once

#include "config.h"
#include "buffers.h"

// Note: When an instance of this class needs to go out of scope,
// Do NOT call delete. Call its "cleanup" method instead.

class TriangleMesh
{
public:
	TriangleMesh(const vk::PhysicalDevice& physicalDevice, const vk::Device& logicalDevice);
	~TriangleMesh();

	vk::Buffer getVertexBuffer();

	void cleanup(const vk::Device& logicalDevice) const;

private:
	vkUtil::BufferData vertexBufferData;
};