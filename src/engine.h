#pragma once

#include "config.h"

#include "frame.h"

#include "scene.h"

#include "SceneData.h"

class Engine
{
public:
	Engine(int width, int height, GLFWwindow* window, const char* appName, bool debugMode);

	~Engine();

	void render(Scene* scene);

private:
	// TODO: Update variable/function naming conventions to be more organized and consistent

	bool debugMode;

	// glfw window params
	int width;
	int height;
	GLFWwindow* window;

	// Instance related variables
	// vulkan instance
	vk::Instance instance{ nullptr };

	// Debug Callback
	vk::DebugUtilsMessengerEXT debugMessenger{ nullptr };

	// Dynamic Instance dispatcher
	// Loads in "instance procedure address"
	// basically, we can create a function pointer to getInstanceProcAddr using this
	// getInstanceProcAddr is used to load in the function to create or destroy the debug messenger
	// We can create this at any point after the creation of the vulkan instance
	vk::DispatchLoaderDynamic dldi;
	vk::SurfaceKHR surface;


	// Device related variables
	vk::PhysicalDevice physicalDevice{ nullptr };
	vk::Device device{ nullptr };
	vk::Queue graphicsQueue{ nullptr };
	vk::Queue presentQueue{ nullptr };
	uint32_t graphicsQueueFamilyIdx;

	// Swapchain
	vk::SwapchainKHR swapchain;
	std::vector<vkUtil::SwapchainFrame> swapchainFrames;
	vk::Format swapchainFormat;
	vk::Extent2D swapchainExtent;
	uint32_t minImageCount;
	uint32_t imageCount;


	// general
	const char *appName;

	// pipeline-related variables
	vk::PipelineLayout layout;
	vk::RenderPass renderPass;
	vk::Pipeline pipeline;

	// command-related variables
	vk::CommandPool commandPool;
	vk::CommandBuffer mainCommandBuffer;

	// sync-related variables
	int maxFramesInFlight, frameNum;

	// Descriptor-related variables
	vk::DescriptorSetLayout descriptorSetLayout;
	vk::DescriptorPool descriptorPool;

	// assets
	SceneData* sceneData;

	// Imgui variables
	vk::DescriptorPool imguiDescriptorPool;
	vk::RenderPass imguiRenderPass;
	// TODO: Make more efficient. Store as array?
	vk::CommandPool imguiMainCommandPool;

	// instance setup
	void make_instance();

	// device setup
	void make_swapchain();
	void recreate_swapchain();
	void make_device();

	// pipeline setup
	void make_descriptor_set_layout();
	void make_pipeline();

	void make_framebuffers();
	void make_frame_resources();
	void finalize_setup();

	void make_assets();
	void prepare_scene(const vk::CommandBuffer& commandBuffer);
	void prepare_frame(const uint32_t imageIndex);

	void record_draw_commands(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);


	// ImGui Helpers
	void init_imgui();
	void create_imgui_descriptor_pool();
	void create_imgui_renderpass();
	vk::CommandPool createImguiCommandPool(vk::CommandPoolCreateFlags flags);
	std::vector<vk::CommandBuffer> createCommandBuffers(uint32_t commandBufferCount, vk::CommandPool& commandPool);

	// cleanup
	void cleanup_imgui();
	void cleanup_swapchain();
	void cleanup_pipeline();
};