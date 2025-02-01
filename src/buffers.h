#pragma once

#include "config.h"

namespace vkUtil
{
	struct BufferInput
	{
		size_t size;
		vk::BufferUsageFlags usage;
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;
	};

	struct BufferData
	{
		vk::Buffer buffer;
		vk::DeviceMemory bufferMemory;
	};

	uint32_t find_memory_type_idx(vk::PhysicalDevice physicalDevice,
		uint32_t supportedMemoryIndices,
		vk::MemoryPropertyFlags requestedProperties);

	void allocateBufferMemory(BufferData& bufferData, const BufferInput& input);

	BufferData create_buffer(const BufferInput& input);
}