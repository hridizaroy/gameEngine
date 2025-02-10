#pragma once

#include "config.h"
#include "shaders.h"
#include "render_structs.h"
#include "mesh.h"

namespace vkInit
{
	struct GraphicsPipelineInBundle
	{
		vk::Device device;
		std::string vertexFilepath;
		std::string fragmentFilepath;
		vk::Extent2D swapchainExtent;
		vk::Format swapchainImageFormat;
		vk::DescriptorSetLayout descriptorSetLayout;
	};

	struct GraphicsPipelineOutBundle
	{
		vk::PipelineLayout layout;
		vk::RenderPass renderpass;
		vk::Pipeline pipeline;
	};


	vk::PipelineLayout make_pipeline_layout(const vk::Device& device,
		const vk::DescriptorSetLayout& descriptorSetLayout, bool debug)
	{
		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo.flags = vk::PipelineLayoutCreateFlags();
		layoutInfo.setLayoutCount = 1; // Descriptor set layout
		layoutInfo.pSetLayouts = &descriptorSetLayout;

		// Push constants
		layoutInfo.pushConstantRangeCount = 1;
		vk::PushConstantRange pushConstantInfo;
		pushConstantInfo.offset = 0;
		pushConstantInfo.size = sizeof(vkUtil::ObjectData);
		pushConstantInfo.stageFlags = vk::ShaderStageFlagBits::eVertex;
		layoutInfo.pPushConstantRanges = &pushConstantInfo;

		try
		{
			return device.createPipelineLayout(layoutInfo);
		}
		catch (vk::SystemError err)
		{
			if (debug)
			{
				std::cout << "Failed to create pipeline layout :/" << std::endl;
			}

			return nullptr;
		}
	}


	vk::RenderPass make_renderpass(vk::Device device, vk::Format swapchainImageFormat, bool debug)
	{
		vk::AttachmentDescription colorAttachment = {};
		colorAttachment.flags = vk::AttachmentDescriptionFlags();
		colorAttachment.format = swapchainImageFormat;
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;

		// changes for imgui
		//colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
		colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;


		vk::AttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0; // index for color attachment
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;


		vk::SubpassDescription subpass = {};
		subpass.flags = vk::SubpassDescriptionFlags();
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		
		vk::RenderPassCreateInfo renderpassInfo = {};
		renderpassInfo.flags = vk::RenderPassCreateFlags();
		renderpassInfo.attachmentCount = 1;
		renderpassInfo.pAttachments = &colorAttachment;
		renderpassInfo.subpassCount = 1;
		renderpassInfo.pSubpasses = &subpass;

		try
		{
			return device.createRenderPass(renderpassInfo);
		}
		catch (vk::SystemError err)
		{
			if (debug)
			{
				std::cout << "Failed to create renderpass!" << std::endl;
			}

			return nullptr;
		}
	}


	GraphicsPipelineOutBundle make_graphics_pipeline(
		const GraphicsPipelineInBundle& specification,
		bool debug)
	{
		vk::GraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.flags = vk::PipelineCreateFlags();

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

		// Vertex Input
		uint32_t binding = 0;
		vk::VertexInputBindingDescription bindingDesc = vkMesh::getBasicVertexBindingDesc(binding);
		std::array<vk::VertexInputAttributeDescription, 2> attrDesc = vkMesh::getBasicAttrDesc(binding);

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = 2;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
		vertexInputInfo.pVertexAttributeDescriptions = attrDesc.data();

		pipelineInfo.pVertexInputState = &vertexInputInfo;

		// Input Assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
		inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;


		// Vertex shader
		if (debug)
		{
			std::cout << "Creating vertex shader module..." << std::endl;
		}

		vk::ShaderModule vertexShader = vkUtil::createModule(specification.vertexFilepath, specification.device, debug);
		vk::PipelineShaderStageCreateInfo vertexShaderInfo = {};
		vertexShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
		vertexShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
		vertexShaderInfo.module = vertexShader;
		vertexShaderInfo.pName = "main";
		shaderStages.push_back(vertexShaderInfo);


		// Viewport and Scissor
		vk::Viewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(specification.swapchainExtent.width);
		viewport.height = static_cast<float>(specification.swapchainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vk::Rect2D scissor = {};
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent = specification.swapchainExtent;

		vk::PipelineViewportStateCreateInfo viewportState = {};
		viewportState.flags = vk::PipelineViewportStateCreateFlags();
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		pipelineInfo.pViewportState = &viewportState;


		// Rasterizer
		vk::PipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.flags = vk::PipelineRasterizationStateCreateFlags();
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = vk::PolygonMode::eFill;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = vk::CullModeFlagBits::eBack;
		rasterizer.frontFace = vk::FrontFace::eClockwise;
		rasterizer.depthBiasEnable = VK_FALSE;
		pipelineInfo.pRasterizationState = &rasterizer;


		// Fragment shader
		vk::ShaderModule fragmentShader = vkUtil::createModule(specification.fragmentFilepath, specification.device, debug);
		vk::PipelineShaderStageCreateInfo fragmentShaderInfo = {};
		fragmentShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
		fragmentShaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
		fragmentShaderInfo.module = fragmentShader;
		fragmentShaderInfo.pName = "main";
		shaderStages.push_back(fragmentShaderInfo);

		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();


		// Multisampling data
		vk::PipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.flags = vk::PipelineMultisampleStateCreateFlags();
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
		pipelineInfo.pMultisampleState = &multisampling;


		// Color Blend
		vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		colorBlendAttachment.blendEnable = VK_FALSE;

		vk::PipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.flags = vk::PipelineColorBlendStateCreateFlags();
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = vk::LogicOp::eCopy;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;
		pipelineInfo.pColorBlendState = &colorBlending;


		// Pipeline layout
		if (debug)
		{
			std::cout << "Create Pipeline Layout" << std::endl;
		}
		vk::PipelineLayout layout = make_pipeline_layout(specification.device,
										specification.descriptorSetLayout, debug);
		pipelineInfo.layout = layout;


		// Renderpass
		if (debug)
		{
			std::cout << "Creating renderpass..." << std::endl;
		}

		vk::RenderPass renderpass = make_renderpass(specification.device, specification.swapchainImageFormat, debug);
		pipelineInfo.renderPass = renderpass;


		// Extras
		pipelineInfo.basePipelineHandle = nullptr;


		// Create the pipeline
		if (debug)
		{
			std::cout << "Creating Graphics Pipeline..." << std::endl;
		}

		vk::Pipeline graphicsPipeline;
		try
		{
			graphicsPipeline = (specification.device.createGraphicsPipeline(nullptr, pipelineInfo)).value;
		}
		catch (vk::SystemError err)
		{
			if (debug)
			{
				std::cout << "Failed to create Graphics Pipeline :/" << std::endl;
			}
		}


		GraphicsPipelineOutBundle output = {};
		output.layout = layout;
		output.renderpass = renderpass;
		output.pipeline = graphicsPipeline;
		
		// cleanup
		specification.device.destroyShaderModule(vertexShader);
		specification.device.destroyShaderModule(fragmentShader);


		return output;
	}
}