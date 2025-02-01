#include "buffers.h"

namespace vkUtil
{
	uint32_t find_memory_type_idx(vk::PhysicalDevice physicalDevice,
		uint32_t supportedMemoryIndices,
		vk::MemoryPropertyFlags requestedProperties)
	{
		vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

		for (uint32_t ii = 0; ii < memoryProperties.memoryTypeCount; ii++)
		{
			bool supported = ((supportedMemoryIndices >> ii) & 1) == 1;

			if (supported)
			{
				bool sufficient = (
					(memoryProperties.memoryTypes[ii].propertyFlags & requestedProperties)
					== requestedProperties);

				if (sufficient)
				{
					return ii;
				}
			}
		}

		throw std::exception("No suitable memory type found!");
	}

	void allocateBufferMemory(BufferData& bufferData, const BufferInput& input)
	{
		vk::MemoryRequirements requirements =
			input.logicalDevice.getBufferMemoryRequirements(bufferData.buffer);

		vk::MemoryAllocateInfo allocInfo;
		allocInfo.memoryTypeIndex = find_memory_type_idx(input.physicalDevice,
			requirements.memoryTypeBits,
			// Host visible = we can write to it directly
			// Host coherent = Write operation happens right on the location,
			// we don't have to worry about sync
			vk::MemoryPropertyFlagBits::eHostVisible
			| vk::MemoryPropertyFlagBits::eHostCoherent);
		allocInfo.allocationSize = input.size;

		bufferData.bufferMemory = input.logicalDevice.allocateMemory(allocInfo);
		input.logicalDevice.bindBufferMemory(bufferData.buffer, bufferData.bufferMemory, 0);
	}

	BufferData create_buffer(const BufferInput& input)
	{
		vk::BufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.flags = vk::BufferCreateFlags();
		bufferCreateInfo.size = input.size;
		bufferCreateInfo.usage = input.usage;
		bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

		BufferData bufferData;
		bufferData.buffer = input.logicalDevice.createBuffer(bufferCreateInfo);

		allocateBufferMemory(bufferData, input);

		return bufferData;
	}
}