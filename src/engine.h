﻿#pragma once

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
	vk::SwapchainKHR swapchain;
	std::vector<vkUtil::SwapchainFrame> swapchainFrames;
	vk::Format swapchainFormat;
	vk::Extent2D swapchainExtent;


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

	// assets
	SceneData* sceneData;

	// instance setup
	void make_instance();

	// device setup
	void make_swapchain();
	void recreate_swapchain();
	void make_device();

	// pipeline setup
	void make_pipeline();

	void make_framebuffers();
	void make_sync_objects();
	void finalize_setup();

	void make_assets();
	void prepare_scene(const vk::CommandBuffer& commandBuffer);

	void record_draw_commands(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);

	// cleanup
	void cleanup_swapchain();
	void cleanup_pipeline();
};