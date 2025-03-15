#pragma once

#include "config.h"
#include "buffers.h"
#include "Entity.h"
#include "scene.h"

namespace vkUtil
{
	struct UBOData
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 viewProjection;
	};

	template <typename T>
	struct BufferHelper
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
		BufferHelper<glm::mat4>* modelUniform;
		BufferHelper<Shape>* shapeUniform;
		BufferHelper<glm::vec4>* shapeParamUniform;
						
						
		// resource descriptors
		vk::DescriptorBufferInfo uniformBufferDescriptor;
		vk::DescriptorBufferInfo modelBufferDescriptor;
		vk::DescriptorBufferInfo shapeBufferDescriptor; 
		vk::DescriptorBufferInfo shapeParamBufferDescriptor; 

		vk::DescriptorSet descriptorSet;

		void make_descriptor_resources(
			const vk::Device& logicalDevice,
			vk::PhysicalDevice& physicalDevice, 
			Scene* scene)
		{

			// NOTE: The recreation of the huge arrays is a giant
			//		 toll on the computer. Need to optimize where
			//		 we do not need to recreate array unless 
			//		 necessary 




			// Generic input that can be modified for buffers in this function 
			vkUtil::BufferInput input;
			input.logicalDevice = logicalDevice;
			input.physicalDevice = physicalDevice;
			input.size = sizeof(UBOData);
			input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
			input.memoryProperties = vk::MemoryPropertyFlagBits::eHostCoherent
				| vk::MemoryPropertyFlagBits::eHostVisible;




			{ // Camera view, proj, viewProj
				camDataBuffer = create_buffer(input);

				camDataWriteLocation = logicalDevice.mapMemory(camDataBuffer.bufferMemory,
					0, input.size);
			}			
			

			// Storage buffer
			// TODO: Should we avoid hard coding the "1024"
			size_t maxBufferSize = 1024;

			// Changing buffer input for storage buffers 
			input.usage = vk::BufferUsageFlagBits::eStorageBuffer;
			
			
			{ // Model Uniform Created 

				// Size
				input.size = maxBufferSize * sizeof(glm::mat4);

				modelUniform = new BufferHelper<glm::mat4>();
				modelUniform->buffer = create_buffer(input);

				modelUniform->bufferWriteLocation = logicalDevice.mapMemory(modelUniform->buffer.bufferMemory,
					0, input.size);

				// Initialize <maxBufferSize> identity matrices
				modelUniform->data.resize(maxBufferSize);
			}
			
			{ // Shapes 

				// Size
				input.size = (scene->shapeEntities.size() + 1) * sizeof(Shape);

				shapeUniform = new BufferHelper<Shape>();
				shapeUniform->buffer = create_buffer(input);
				shapeUniform->bufferWriteLocation = logicalDevice.mapMemory(shapeUniform->buffer.bufferMemory,
					0, input.size);
				
				shapeUniform->data.resize((scene->shapeEntities.size() + 1));
			}



			uint32_t paramCount = 0;
			{ // Shape parameters 

				// Match shape entities 
				for (auto shape : scene->shapeEntities)
				{
					//scene->GetShapeParamCount(shape->shape.shapeInfo.a);
					paramCount += ShapeTypes::GetShapeParamSize(shape->GetShapeID());
				}


				// Size
				input.size = paramCount * sizeof(glm::vec4);

				shapeParamUniform = new BufferHelper<glm::vec4>();
				shapeParamUniform->buffer = create_buffer(input);
				shapeParamUniform->bufferWriteLocation = logicalDevice.mapMemory(shapeParamUniform->buffer.bufferMemory,
					0, input.size);

				// We need to find out how many parameters where each entity
				// can have some arbitrary amount 

				

				printf("Total parameters: %i \n", paramCount);

				// Note: This might need additional params for extra spacing 
				shapeParamUniform->data.resize(paramCount);
			}




			// --- Initialize Buffer Info --- 


			// View, proj, and viewproj 
			uniformBufferDescriptor.buffer = camDataBuffer.buffer;
			uniformBufferDescriptor.offset = 0;
			uniformBufferDescriptor.range = sizeof(UBOData);

			// Models 
			modelBufferDescriptor.buffer = modelUniform->buffer.buffer;
			modelBufferDescriptor.offset = 0;
			modelBufferDescriptor.range = maxBufferSize * sizeof(glm::mat4);

			// Shapes 
			shapeBufferDescriptor.buffer = shapeUniform->buffer.buffer;
			shapeBufferDescriptor.offset = 0;
			shapeBufferDescriptor.range = (scene->shapeEntities.size() + 1) * sizeof(Shape);

			// Shape Parameters 
			shapeParamBufferDescriptor.buffer = shapeParamUniform->buffer.buffer;
			shapeParamBufferDescriptor.offset = 0;
			shapeParamBufferDescriptor.range = paramCount * sizeof(glm::vec4);
		}


		void fill_descriptor_set(const vk::Device& logicalDevice)
		{
			// Here we set up our descriptor set that defines the data we will be sending 

			// TODO:
			//	[ ] Update dstBinding to update per cout
			//	[ ] Combine both the shape and their parameters 
			//		in one array and have offset? 


			{ // View, proj, and viewproj 
				vk::WriteDescriptorSet writeInfo;
				writeInfo.descriptorCount = 1;
				writeInfo.descriptorType = vk::DescriptorType::eUniformBuffer;
				writeInfo.dstSet = descriptorSet;
				writeInfo.dstBinding = 0;
				writeInfo.dstArrayElement = 0;
				writeInfo.pBufferInfo = &uniformBufferDescriptor;

				logicalDevice.updateDescriptorSets(writeInfo, nullptr); // On 4th continue this breaks 
			}
			
			{ // Model
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

			{ // Shapes 
				vk::WriteDescriptorSet writeInfo;
				writeInfo.descriptorCount = 1;
				writeInfo.descriptorType = vk::DescriptorType::eStorageBuffer;
				writeInfo.dstSet = descriptorSet;
				writeInfo.dstBinding = 2;

				// byte offset within binding for inline uniform blocks
				writeInfo.dstArrayElement = 0;
				writeInfo.pBufferInfo = &shapeBufferDescriptor;

				logicalDevice.updateDescriptorSets(writeInfo, nullptr);
			}

			{ // Shape Params
				vk::WriteDescriptorSet writeInfo;
				writeInfo.descriptorCount = 1;
				writeInfo.descriptorType = vk::DescriptorType::eStorageBuffer;
				writeInfo.dstSet = descriptorSet;
				writeInfo.dstBinding = 3;

				// byte offset within binding for inline uniform blocks
				writeInfo.dstArrayElement = 0;
				writeInfo.pBufferInfo = &shapeParamBufferDescriptor;

				logicalDevice.updateDescriptorSets(writeInfo, nullptr);
			}
		}

		~SwapchainFrame()
		{
			delete modelUniform;
			delete shapeUniform;
			delete shapeParamUniform;
		}
	};
}