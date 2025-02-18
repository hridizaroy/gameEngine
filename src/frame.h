#pragma once

#include "config.h"
#include "buffers.h"

namespace vkUtil
{
	struct UBOData
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 viewProjection;
	};

	struct SwapchainFrame
	{
		// swapchain
		vk::Image image;
		vk::ImageView imageView;
		vk::Framebuffer frameBuffer;

		vk::CommandBuffer commandBuffer;

		// imgui
		vk::Framebuffer imguiFrameBuffer;
		vk::CommandBuffer imguiCommandBuffer;

		// sync-related variables
		vk::Semaphore imageAvailable, renderFinished;
		vk::Fence inFlight;

		// resources
		UBOData camData;
		BufferData camDataBuffer;
		void* camDataWriteLocation;
		std::vector<glm::mat4> modelTransforms;
		BufferData modelBuffer;
		void* modelBufferWriteLocation;

		// resource descriptors
		vk::DescriptorBufferInfo uniformBufferDescriptor;
		vk::DescriptorSet descriptorSet;

		void make_descriptor_resources(const vk::Device& logicalDevice,
			vk::PhysicalDevice& physicalDevice)
		{
			vkUtil::BufferInput input;
			input.logicalDevice = logicalDevice;
			input.physicalDevice = physicalDevice;
			input.size = sizeof(UBOData);
			input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
			input.memoryProperties = vk::MemoryPropertyFlagBits::eHostCoherent
				| vk::MemoryPropertyFlagBits::eHostVisible;

			camDataBuffer = create_buffer(input);

			camDataWriteLocation = logicalDevice.mapMemory(camDataBuffer.bufferMemory,
									0, sizeof(UBOData));

			uniformBufferDescriptor.buffer = camDataBuffer.buffer;
			uniformBufferDescriptor.offset = 0;
			uniformBufferDescriptor.range = sizeof(UBOData);
		}

		void fill_descriptor_set(const vk::Device& logicalDevice)
		{
			vk::WriteDescriptorSet writeInfo;
			writeInfo.descriptorCount = 1;
			writeInfo.descriptorType = vk::DescriptorType::eUniformBuffer;
			writeInfo.dstSet = descriptorSet;
			writeInfo.dstBinding = 0;
			writeInfo.dstArrayElement = 0;
			writeInfo.pBufferInfo = &uniformBufferDescriptor;

			logicalDevice.updateDescriptorSets(writeInfo, nullptr);
		}
	};
}