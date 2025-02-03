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

	void allocate_buffer_memory(BufferData& bufferData, const BufferInput& input)
	{
		vk::MemoryRequirements requirements =
			input.logicalDevice.getBufferMemoryRequirements(bufferData.buffer);

		vk::MemoryAllocateInfo allocInfo;
		allocInfo.memoryTypeIndex = find_memory_type_idx(input.physicalDevice,
			requirements.memoryTypeBits,
			input.memoryProperties);
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

		allocate_buffer_memory(bufferData, input);

		return bufferData;
	}

	void copy_buffer(const BufferData& srcBufferData, const BufferData& dstBufferData,
		const vk::DeviceSize& size, const vk::Queue& queue, const vk::CommandBuffer& commandBuffer)
	{
		commandBuffer.reset();

		vk::CommandBufferBeginInfo beginInfo{};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		commandBuffer.begin(beginInfo);

		vk::BufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		
		commandBuffer.copyBuffer(srcBufferData.buffer, dstBufferData.buffer, 1, &copyRegion);

		commandBuffer.end();

		vk::SubmitInfo submitInfo{};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		queue.submit(1, &submitInfo, nullptr);
		queue.waitIdle();
	}
}