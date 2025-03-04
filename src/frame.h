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

	template <typename T>
	struct Uniform
	{
		std::vector<T> data;
		BufferData buffer;
		void* bufferWriteLocation;
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

		// TODO: Can we make this an array?
		Uniform<glm::mat4>* modelUniform; 
		Uniform<float> shapeUniform;
		Uniform<float> shapeParamUniform; 
						
						
		// resource descriptors
		vk::DescriptorBufferInfo uniformBufferDescriptor;
		vk::DescriptorBufferInfo modelBufferDescriptor;

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
									0, input.size);

			// Storage buffer
			// TODO: Should we avoid hard coding the "1024"
			size_t maxBufferSize = 1024;

			input.size = maxBufferSize * sizeof(glm::mat4);
			input.usage = vk::BufferUsageFlagBits::eStorageBuffer;
			modelUniform = new Uniform<glm::mat4>();
			modelUniform->buffer = create_buffer(input);

			modelUniform->bufferWriteLocation = logicalDevice.mapMemory(modelUniform->buffer.bufferMemory,
				0, input.size);

			// Initialize <maxBufferSize> identity matrices
			modelUniform->data.resize(maxBufferSize);

			uniformBufferDescriptor.buffer = camDataBuffer.buffer;
			uniformBufferDescriptor.offset = 0;
			uniformBufferDescriptor.range = sizeof(UBOData);

			modelBufferDescriptor.buffer = modelUniform->buffer.buffer;
			modelBufferDescriptor.offset = 0;
			modelBufferDescriptor.range = maxBufferSize * sizeof(glm::mat4);


			// Shapes 



			// Shape Parameters 
		}

		void fill_descriptor_set(const vk::Device& logicalDevice)
		{
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
			
			{
				vk::WriteDescriptorSet writeInfo;
				writeInfo.descriptorCount = 1;
				writeInfo.descriptorType = vk::DescriptorType::eStorageBuffer;
				writeInfo.dstSet = descriptorSet;
				writeInfo.dstBinding = 1;

				// byte offset within binding for inline uniform blocks
				writeInfo.dstArrayElement = 0;
				writeInfo.pBufferInfo = &modelBufferDescriptor;

				logicalDevice.updateDescriptorSets(writeInfo, nullptr);
			}
		}

		~SwapchainFrame()
		{
			delete modelUniform;
		}
	};
}