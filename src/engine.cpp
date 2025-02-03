#include "engine.h"
#include "instance.h"
#include "logging.h"
#include "device.h"
#include "swapchain.h"
#include "pipeline.h"
#include "framebuffer.h"
#include "commands.h"
#include "sync.h"


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

	maxFramesInFlight = static_cast<int>(swapchainFrames.size());
}

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
	make_sync_objects();

	vkInit::commandBufferInputChunk commandBufferInput = { device, commandPool, swapchainFrames };
	vkInit::make_frame_command_buffers(commandBufferInput, debugMode);

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

	make_swapchain();
	
	frameNum = 0;
}

void Engine::make_pipeline()
{
	vkInit::GraphicsPipelineInBundle specification = {};
	specification.device = device;
	specification.vertexFilepath = "./shaders/vertex.spv";
	specification.fragmentFilepath = "./shaders/fragment.spv";
	specification.swapchainExtent = swapchainExtent;
	specification.swapchainImageFormat = swapchainFormat;

	// TODO: Handle File IO errors
	vkInit::GraphicsPipelineOutBundle output = vkInit::make_graphics_pipeline(specification, debugMode);
	layout = output.layout;
	renderPass = output.renderpass;
	pipeline = output.pipeline;
}

void Engine::make_framebuffers()
{
	vkInit::framebufferInput framebufferInput;
	framebufferInput.device = device;
	framebufferInput.renderpass = renderPass;
	framebufferInput.swapchainExtent = swapchainExtent;

	vkInit::make_framebuffers(framebufferInput, swapchainFrames, debugMode);
}

void Engine::make_sync_objects()
{
	for (vkUtil::SwapchainFrame& frame : swapchainFrames)
	{
		frame.imageAvailable = vkInit::make_semaphore(device, debugMode);
		frame.renderFinished = vkInit::make_semaphore(device, debugMode);
		frame.inFlight = vkInit::make_fence(device, debugMode);
	}
}

void Engine::finalize_setup()
{
	make_framebuffers();

	commandPool = vkInit::make_command_pool(device, physicalDevice, surface, debugMode);

	vkInit::commandBufferInputChunk commandBufferInput = { device, commandPool, swapchainFrames };
	mainCommandBuffer = vkInit::make_main_command_buffer(commandBufferInput, debugMode);
	vkInit::make_frame_command_buffers(commandBufferInput, debugMode);

	make_sync_objects();
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
			vkUtil::ObjectData objectData;
			objectData.model = model;

			commandBuffer.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(objectData), &objectData);
			commandBuffer.draw(vertexCount, 1, offset, 0);
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
			commandBuffer.draw(vertexCount, 1, offset, 0);
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
			commandBuffer.draw(vertexCount, 1, offset, 0);
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

	record_draw_commands(commandBuffer, imageIndex, scene);

	VkSubmitInfo submitInfo{};

	VkSemaphore waitSemaphores[] = { swapchainFrames[frameNum].imageAvailable };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

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

void Engine::cleanup_swapchain()
{
	for (const auto& frame : swapchainFrames)
	{
		device.destroyImageView(frame.imageView);
		device.destroyFramebuffer(frame.frameBuffer);

		device.destroySemaphore(frame.imageAvailable);
		device.destroySemaphore(frame.renderFinished);
		device.destroyFence(frame.inFlight);
	}

	device.destroySwapchainKHR(swapchain);
}

void Engine::cleanup_pipeline()
{
	device.destroyPipeline(pipeline);
	device.destroyPipelineLayout(layout);
	device.destroyRenderPass(renderPass);
}

Engine::~Engine()
{
	device.waitIdle(); // wait until device is idle

	if (debugMode)
	{
		std::cout << "Bye!\n";
	}

	device.destroyCommandPool(commandPool);

	cleanup_pipeline();

	cleanup_swapchain();

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
