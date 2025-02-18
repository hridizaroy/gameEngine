#include "engine.h"
#include "instance.h"
#include "logging.h"
#include "device.h"
#include "swapchain.h"
#include "pipeline.h"
#include "framebuffer.h"
#include "commands.h"
#include "sync.h"
#include "descriptors.h"

// Imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#define APP_USE_VULKAN_DEBUG_REPORT


Engine::Engine(int width, int height, GLFWwindow* window, const char* appName, bool debugMode)
{
	this->width = width;
	this->height = height;
	this->window = window;
	this->debugMode = debugMode;
	this->appName = appName;

	if (debugMode)
	{
		std::cout << "Creating our Graphics Engine\n";
	}

	make_instance();
	make_device();

	make_descriptor_set_layout();
	make_pipeline();

	finalize_setup();

	make_assets();
}

void Engine::make_instance()
{
	// Create Vulkan instance
	instance = vkInit::make_instance(debugMode, appName);
	
	// Create dispatch loader to assist with debug messenger
	dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);

	// Create Debug messenger
	if (debugMode)
	{
		debugMessenger = vkInit::make_debug_messenger(instance, dldi);
	}

	// Create surface
	VkSurfaceKHR c_style_surface;
	if (glfwCreateWindowSurface(instance, window, nullptr, &c_style_surface) != VK_SUCCESS)
	{
		if (debugMode)
		{
			std::cout << "Failed to abstract the glfw surface for Vulkan.\n";
		}
	}
	else if (debugMode)
	{
		std::cout << "Successfully abstracted the glfw surface for Vulkan.\n";
	}

	surface = c_style_surface;
}

void Engine::make_swapchain()
{
	vkInit::SwapchainBundle bundle = vkInit::create_swapchain(device, physicalDevice, surface, width, height, debugMode);
	swapchain = bundle.swapchain;
	swapchainFrames = bundle.frames;
	swapchainFormat = bundle.format;
	swapchainExtent = bundle.extent;

	minImageCount = bundle.minImageCount;
	imageCount = bundle.imageCount;

	maxFramesInFlight = static_cast<int>(swapchainFrames.size());
}

// TODO: How to adapt this for Imgui?
void Engine::recreate_swapchain()
{
	// if minimized, wait until our window is reopened
	width = 0;
	height = 0;

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	device.waitIdle();

	cleanup_swapchain();

	make_swapchain();
	make_framebuffers();
	make_frame_resources();

	vkInit::commandBufferInputChunk commandBufferInput = { device, commandPool, swapchainFrames, imguiMainCommandPool };
	vkInit::make_frame_command_buffers(commandBufferInput, debugMode);

	// update imgui imagecount
	ImGui_ImplVulkan_SetMinImageCount(std::max(minImageCount, static_cast<uint32_t>(2)));

	// TODO: Identify potentially redundant steps in the following code
	// Remove these lines if we don't want to make the render adapt to screen resizing
	cleanup_pipeline();
	make_pipeline();
}

void Engine::make_device()
{
	// physical device
	physicalDevice = vkInit::choose_physical_device(instance, debugMode);

	// logical device
	device = vkInit::create_logical_device(physicalDevice, surface, debugMode);

	// Queues
	std::array<vk::Queue, 2> queues = vkInit::get_queue(physicalDevice, device, surface, debugMode);
	graphicsQueue = queues[0];
	presentQueue = queues[1];

	// Get queue family index
	// Required for Imgui
	// TODO: Reduce redundancy
	graphicsQueueFamilyIdx = vkUtil::findQueueFamilies(physicalDevice, surface, debugMode).graphicsFamily.value();

	make_swapchain();
	
	frameNum = 0;
}

void Engine::make_descriptor_set_layout()
{
	vkInit::DescriptorSetLayoutData bindings{};
	//bindings.count = 2;
	bindings.count = 1;

	// Uniform buffer
	bindings.indices.push_back(0);
	bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	bindings.counts.push_back(1);
	bindings.stages.push_back(vk::ShaderStageFlagBits::eVertex);

	// Storage buffer
	//bindings.indices.push_back(1);
	//bindings.types.push_back(vk::DescriptorType::eStorageBuffer);
	//bindings.counts.push_back(1);
	//bindings.stages.push_back(vk::ShaderStageFlagBits::eVertex);


	// Since storage buffer and uniform buffer are used with the same frequency,
	// we are binding them to the same descriptor set

	descriptorSetLayout = vkInit::make_descriptor_set_layout(device, bindings);
}

void Engine::make_pipeline()
{
	vkInit::GraphicsPipelineInBundle specification{};
	specification.device = device;
	specification.vertexFilepath = "./shaders/vertex.spv";
	specification.fragmentFilepath = "./shaders/fragment.spv";
	specification.swapchainExtent = swapchainExtent;
	specification.swapchainImageFormat = swapchainFormat;
	specification.descriptorSetLayout = descriptorSetLayout;

	// TODO: Handle File IO errors
	vkInit::GraphicsPipelineOutBundle output = vkInit::make_graphics_pipeline(specification, debugMode);
	layout = output.layout;
	renderPass = output.renderpass;
	pipeline = output.pipeline;

	// imgui renderpass
	create_imgui_renderpass();
}

void Engine::make_framebuffers()
{
	vkInit::framebufferInput framebufferInput;
	framebufferInput.device = device;
	framebufferInput.renderpass = renderPass;
	framebufferInput.swapchainExtent = swapchainExtent;

	// imgui
	framebufferInput.imguiRenderpass = imguiRenderPass;

	vkInit::make_framebuffers(framebufferInput, swapchainFrames, debugMode);
}

void Engine::make_frame_resources()
{
	vkInit::DescriptorSetLayoutData bindings{};
	bindings.count = 1;
	bindings.types.push_back(vk::DescriptorType::eUniformBuffer);

	descriptorPool = vkInit::make_descriptor_pool(device,
		static_cast<uint32_t>(swapchainFrames.size()), bindings);


	for (vkUtil::SwapchainFrame& frame : swapchainFrames)
	{
		frame.imageAvailable = vkInit::make_semaphore(device, debugMode);
		frame.renderFinished = vkInit::make_semaphore(device, debugMode);
		frame.inFlight = vkInit::make_fence(device, debugMode);

		frame.make_descriptor_resources(device, physicalDevice);

		frame.descriptorSet = vkInit::allocate_descriptor_set(
			device, descriptorPool, descriptorSetLayout);
	}
}

void Engine::finalize_setup()
{
	// imgui
	init_imgui();

	make_framebuffers();

	commandPool = vkInit::make_command_pool(device, physicalDevice, surface, debugMode);

	vkInit::commandBufferInputChunk commandBufferInput = { device, commandPool, swapchainFrames, imguiMainCommandPool };
	mainCommandBuffer = vkInit::make_main_command_buffer(commandBufferInput, debugMode);
	vkInit::make_frame_command_buffers(commandBufferInput, debugMode);

	make_frame_resources();
}

// Vertices need to be in counterclockwise winding order
// (assuming the axis is coming from the screen towards you)
void Engine::make_assets()
{
	sceneData = new SceneData();

	std::vector<float> vertexData = {
		1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -0.05f, 0.0f,
		1.0f, 0.0f, 1.0f, 1.0f, 0.05f, 0.05f, 0.0f,
		1.0f, 0.0f, 1.0f, 1.0f, -0.05f, 0.05f, 0.0f
	};

	sceneData->consume(MeshType::TRIANGLE, vertexData);

	vertexData = {
		0.0f, 1.0f, 0.0f, 1.0f, -0.55f, -0.6f, +0.0f,
		0.0f, 1.0f, 0.0f, 1.0f, -0.75f, -0.7f, +0.0f,
		0.0f, 1.0f, 0.0f, 1.0f, -0.6f, -0.65f, +0.0f,

		0.0f, 1.0f, 0.0f, 1.0f, -0.55f, -0.6f, +0.0f,
		0.0f, 1.0f, 0.0f, 1.0f, -0.9f, -0.5f, +0.0f,
		0.0f, 1.0f, 0.0f, 1.0f, -0.75f, -0.7f, +0.0f,

		0.0f, 1.0f, 0.0f, 1.0f, -0.55f, -0.6f, +0.0f,
		0.0f, 1.0f, 0.0f, 1.0f, -0.8f, +0.9f, +0.0f,
		0.0f, 1.0f, 0.0f, 1.0f, -0.9f, -0.5f, +0.0f
	};

	sceneData->consume(MeshType::PENTAGON, vertexData);

	vertexData = {
		1.0f, 0.0f, 0.0f, 1.0f, +0.8f, +0.9f, +0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, +0.75f, -0.7f, +0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, +0.9f, -0.5f, +0.0f,

		1.0f, 0.0f, 0.0f, 1.0f, +0.8f, +0.9f, +0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, +0.6f, -0.65f, +0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, +0.75f, -0.7f, +0.0f,

		1.0f, 0.0f, 0.0f, 1.0f, +0.8f, +0.9f, +0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, +0.55f, -0.6f, +0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, +0.6f, -0.65f, +0.0f,

		1.0f, 0.0f, 0.0f, 1.0f, +0.8f, +0.9f, +0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, +0.7f, +0.85f, +0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, +0.55f, -0.6f, +0.0f
	};

	sceneData->consume(MeshType::HEXAGON, vertexData);

	FinalizationChunk finalizationChunk{device, physicalDevice, graphicsQueue, mainCommandBuffer};
	sceneData->finalize(finalizationChunk);
}

void Engine::prepare_frame(const uint32_t imageIndex)
{
	glm::vec3 eye{ 0.0f, 0.0f, 2.0f };
	glm::vec3 center{ 0.0f, 0.0f, 0.0f };
	glm::vec3 up{ 0.0f, 0.0f, 1.0f };
	glm::mat4 view{ glm::lookAtRH(eye, center, up) };

	glm::mat4 projection = glm::perspectiveRH(
		glm::radians(45.0f),
		static_cast<float>(swapchainExtent.height) / static_cast<float>(swapchainExtent.width),
		0.1f, 10.0f
	);

	projection[1][1] *= -1.0f;


	swapchainFrames[imageIndex].camData.view = view;
	swapchainFrames[imageIndex].camData.projection = projection;
	swapchainFrames[imageIndex].camData.viewProjection = projection * view;

	memcpy(swapchainFrames[imageIndex].camDataWriteLocation,
		&(swapchainFrames[imageIndex].camData),
		sizeof(vkUtil::UBOData));

	swapchainFrames[imageIndex].fill_descriptor_set(device);
}

void Engine::prepare_scene(const vk::CommandBuffer& commandBuffer)
{
	vk::Buffer vertexBuffers[] = { sceneData->getVertexBuffer() };
	vk::DeviceSize offsets[] = { 0 };

	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
}

void Engine::record_draw_commands(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene)
{
	vk::CommandBufferBeginInfo beginInfo = {};

	try
	{
		commandBuffer.begin(beginInfo);
	}
	catch (vk::SystemError err)
	{
		if (debugMode)
		{
			std::cout << "Failed to begin recording command buffer :/" << std::endl;
		}
	}

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapchainFrames[imageIndex].frameBuffer;
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = swapchainExtent;

	vk::ClearValue clearColor = { std::array<float, 4>{0.2f, 0.1f, 0.9f, 1.0f} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		layout,
		0,
		swapchainFrames[imageIndex].descriptorSet,
		nullptr
	);

	prepare_scene(commandBuffer);

	// Triangle
	std::pair<size_t, size_t> offset_size = sceneData->lookupOffsetSize(MeshType::TRIANGLE);

	size_t offset = offset_size.first;
	size_t vertexCount = offset_size.second;

	if (vertexCount != 0)
	{
		for (glm::vec3& position : scene->trianglePositions)
		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
			vkUtil::ObjectData objectData{};
			objectData.model = model;

			commandBuffer.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(objectData), &objectData);
			commandBuffer.draw(static_cast<uint32_t>(vertexCount), 1, static_cast<uint32_t>(offset), 0);
		}
	}

	// Pentagon
	offset_size = sceneData->lookupOffsetSize(MeshType::PENTAGON);

	offset = offset_size.first;
	vertexCount = offset_size.second;

	if (vertexCount != 0)
	{
		for (glm::vec3& position : scene->pentagonPositions)
		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
			vkUtil::ObjectData objectData;
			objectData.model = model;

			commandBuffer.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(objectData), &objectData);
			commandBuffer.draw(static_cast<uint32_t>(vertexCount), 1, static_cast<uint32_t>(offset), 0);
		}
	}

	// Hexagon
	offset_size = sceneData->lookupOffsetSize(MeshType::HEXAGON);

	offset = offset_size.first;
	vertexCount = offset_size.second;

	if (vertexCount != 0)
	{
		for (glm::vec3& position : scene->hexagonPositions)
		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
			vkUtil::ObjectData objectData;
			objectData.model = model;

			commandBuffer.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(objectData), &objectData);
			commandBuffer.draw(static_cast<uint32_t>(vertexCount), 1, static_cast<uint32_t>(offset), 0);
		}
	}

	/*glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	vkUtil::ObjectData objectData;
	objectData.model = model;
	commandBuffer.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(objectData), &objectData);

	commandBuffer.draw(6, 1, 0, 0)*/;

	commandBuffer.endRenderPass();

	try
	{
		commandBuffer.end();
	}
	catch (vk::SystemError err)
	{
		if (debugMode)
		{
			std::cout << "Failed to finish recording command buffer :/" << std::endl;
		}
	}
}

void Engine::render(Scene* scene)
{
	device.waitForFences(1, &swapchainFrames[frameNum].inFlight, VK_TRUE, UINT64_MAX);
	device.resetFences(1, &swapchainFrames[frameNum].inFlight);

	// Acquire next image
	uint32_t imageIndex;

	// Using C-based functions because we don't want a try/catch overhead
	VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, swapchainFrames[frameNum].imageAvailable, nullptr, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		recreate_swapchain();
		return;
	}

	VkCommandBuffer commandBuffer = swapchainFrames[frameNum].commandBuffer;

	vkResetCommandBuffer(commandBuffer, 0);

	// imgui

	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::ShowDemoWindow();
	ImGui::Begin("Another Window");
	ImGui::Text("Hello from another window!");
	ImGui::End();
	ImGui::Render();

	// Imgui
	{
		vkResetCommandBuffer(swapchainFrames[frameNum].imguiCommandBuffer, 0);

		vk::CommandBufferBeginInfo info{};
		info.flags |= vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		swapchainFrames[frameNum].imguiCommandBuffer.begin(info);
	}
	{
		vk::RenderPassBeginInfo info{};
		info.renderPass = imguiRenderPass;
		info.framebuffer = swapchainFrames[imageIndex].imguiFrameBuffer;
		info.renderArea.extent.width = swapchainExtent.width;
		info.renderArea.extent.height = swapchainExtent.height;
		info.clearValueCount = 1;

		vk::ClearValue clearColor = { std::array<float, 4>{0.9f, 0.1f, 0.1f, 1.0f} };
		info.pClearValues = &clearColor;

		swapchainFrames[frameNum].imguiCommandBuffer.beginRenderPass(info, vk::SubpassContents::eInline);
	}

	// Record Imgui Draw Data and draw funcs into command buffer
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), swapchainFrames[frameNum].imguiCommandBuffer);
	// Submit command buffer
	vkCmdEndRenderPass(swapchainFrames[frameNum].imguiCommandBuffer);
	vkEndCommandBuffer(swapchainFrames[frameNum].imguiCommandBuffer);


	prepare_frame(imageIndex);

	record_draw_commands(commandBuffer, imageIndex, scene);

	std::array<VkCommandBuffer, 2> submitCommandBuffers =
	{ commandBuffer, swapchainFrames[imageIndex].imguiCommandBuffer };

	VkSubmitInfo submitInfo{};

	VkSemaphore waitSemaphores[] = { swapchainFrames[frameNum].imageAvailable };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	//submitInfo.commandBufferCount = 1;
	submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommandBuffers.size());
	//submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.pCommandBuffers = submitCommandBuffers.data();

	VkSemaphore signalSemaphores[] = { swapchainFrames[frameNum].renderFinished };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, swapchainFrames[frameNum].inFlight);

	if (result != VK_SUCCESS)
	{
		if (debugMode)
		{
			std::cout << "Failed to submit draw command buffer :/" << std::endl;
		}
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapchains[] = { swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		recreate_swapchain();
		return;
	}

	frameNum = (frameNum + 1) % maxFramesInFlight;
}

// TODO: Should this go in descriptors.h?
void Engine::create_imgui_descriptor_pool()
{
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE },
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 0;

	for (VkDescriptorPoolSize& pool_size : pool_sizes)
		pool_info.maxSets += pool_size.descriptorCount;
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	imguiDescriptorPool = device.createDescriptorPool(pool_info);
}


// TODO: Should this be in pipeline.h?
void Engine::create_imgui_renderpass()
{
	vk::AttachmentDescription attachment = {};
	attachment.format = swapchainFormat;
	attachment.samples = vk::SampleCountFlagBits::e1;
	attachment.loadOp = vk::AttachmentLoadOp::eLoad;
	attachment.storeOp = vk::AttachmentStoreOp::eStore;
	attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachment.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
	attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference color_attachment = {};
	color_attachment.attachment = 0;
	color_attachment.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment;

	vk::SubpassDependency dependency = {};
	dependency.srcSubpass = vk::SubpassExternal;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;	
	dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependency.srcAccessMask = vk::AccessFlagBits::eNone;
	dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

	vk::RenderPassCreateInfo info = {};
	info.flags = vk::RenderPassCreateFlags();
	info.attachmentCount = 1;
	info.pAttachments = &attachment;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;
	info.dependencyCount = 1;
	info.pDependencies = &dependency;

	try
	{
		imguiRenderPass = device.createRenderPass(info);
	}
	catch (vk::SystemError err)
	{
		if (debugMode)
		{
			std::cout << "Failed to create Imgui renderpass!" << std::endl;
		}
	}
}

// temporarily here
vk::CommandPool Engine::createImguiCommandPool(vk::CommandPoolCreateFlags flags)
{
	vk::CommandPoolCreateInfo commandPoolCreateInfo {};
	commandPoolCreateInfo.flags = flags;
	commandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIdx;

	try
	{
		return device.createCommandPool(commandPoolCreateInfo);
	}
	catch (vk::SystemError err)
	{
		if (debugMode)
		{
			std::cout << "Failed to create Imgui Command Pool :/" << std::endl;
		}

		return nullptr;
	}
}

std::vector<vk::CommandBuffer> Engine::createCommandBuffers(uint32_t commandBufferCount, vk::CommandPool& commandPool)
{
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo {};
	commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.commandBufferCount = commandBufferCount;

	return device.allocateCommandBuffers(commandBufferAllocateInfo);
}



void Engine::init_imgui()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Command pool and buffers
	imguiMainCommandPool = createImguiCommandPool(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	create_imgui_descriptor_pool();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance;
	init_info.PhysicalDevice = physicalDevice;
	init_info.Device = device;
	init_info.QueueFamily = graphicsQueueFamilyIdx;
	init_info.Queue = graphicsQueue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = imguiDescriptorPool;
	init_info.RenderPass = imguiRenderPass;
	init_info.Subpass = 0;
	init_info.MinImageCount = std::max(minImageCount, static_cast<uint32_t>(2));
	init_info.ImageCount = imageCount;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;

	// TODO: It's good practice to actually have our own error handling logic
	// So we should probably add that at some point
	init_info.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init_info);
}

void Engine::cleanup_imgui()
{
	// Resources to destroy when the program ends
	device.destroyCommandPool(imguiMainCommandPool);

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	device.destroyDescriptorPool(imguiDescriptorPool);
}

void Engine::cleanup_swapchain()
{
	for (const auto& frame : swapchainFrames)
	{
		device.destroyImageView(frame.imageView);
		device.destroyFramebuffer(frame.frameBuffer);

		// imgui
		device.destroyFramebuffer(frame.imguiFrameBuffer);

		//device.freeCommandBuffers(commandPool, 1, &frame.commandBuffer);

		device.destroySemaphore(frame.imageAvailable);
		device.destroySemaphore(frame.renderFinished);
		device.destroyFence(frame.inFlight);

		device.unmapMemory(frame.camDataBuffer.bufferMemory);
		device.freeMemory(frame.camDataBuffer.bufferMemory);
		device.destroyBuffer(frame.camDataBuffer.buffer);
	}

	device.destroySwapchainKHR(swapchain);

	device.destroyDescriptorPool(descriptorPool);

	cleanup_imgui();
}

void Engine::cleanup_pipeline()
{
	device.destroyPipeline(pipeline);
	device.destroyPipelineLayout(layout);
	device.destroyRenderPass(renderPass);

	device.destroyRenderPass(imguiRenderPass);
}

Engine::~Engine()
{
	device.waitIdle(); // wait until device is idle

	if (debugMode)
	{
		std::cout << "Bye!\n";
	}

	cleanup_pipeline();

	cleanup_swapchain();

	device.destroyCommandPool(commandPool);

	device.destroyDescriptorSetLayout(descriptorSetLayout);

	sceneData->cleanup(device);

	device.destroy();

	instance.destroySurfaceKHR(surface);
	if (debugMode)
	{
		instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
	}

	instance.destroy();

	glfwTerminate();
}
