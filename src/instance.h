#pragma once
#include "config.h"

// namespace for creating functions etc.
namespace vkInit
{
	// Function to check if our extensions and layers are supported
	bool supported(std::vector<const char*>& extensions, std::vector<const char*>& layers, bool debug)
	{
		// Checking extension support
		std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();

		if (debug)
		{
			std::cout << "Device can support the following extensions:\n";

			for (vk::ExtensionProperties extension : supportedExtensions)
			{
				std::cout << "\t" << extension.extensionName << "\n";
			}
		}

		
		// Go through each extension and check if it's supported
		bool found;
		for (const char* extension : extensions)
		{
			found = false;

			for (vk::ExtensionProperties supportedExtension : supportedExtensions)
			{
				if (strcmp(extension, supportedExtension.extensionName) == 0)
				{
					found = true;

					if (debug)
					{
						std::cout << "Extension \"" << extension << "\" is supported\n";
					}
				}
			}

			if (!found)
			{
				if (debug)
				{
					std::cout << "Extension \"" << extension << "\" is not supported\n";
				}

				return false;
			}
		}


		// Checking layer support
		std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();

		if (debug)
		{
			std::cout << "Device can support the following layers:\n";

			for (vk::LayerProperties layer : supportedLayers)
			{
				std::cout << "\t" << layer.layerName << "\n";
			}
		}

		// Go through each layer and check if it's supported
		for (const char* layer : layers)
		{
			found = false;

			for (vk::LayerProperties supportedLayer : supportedLayers)
			{
				if (strcmp(layer, supportedLayer.layerName) == 0)
				{
					found = true;

					if (debug)
					{
						std::cout << "Layer \"" << layer << "\" is supported\n";
					}
				}
			}

			if (!found)
			{
				if (debug)
				{
					std::cout << "Layer \"" << layer << "\" is not supported\n";
				}

				return false;
			}
		}


		return true;
	}


	// Function to create Vulkan Instance
	vk::Instance make_instance(bool debug, const char* appName)
	{
		if (debug)
		{
			std::cout << "Creating an instance...\n";
		}


		// query our system about what vulkan version it'll support up to
		uint32_t version{ 0 };
		vkEnumerateInstanceVersion(&version);

		if (debug)
		{
			std::cout << "System can support Vulkan variant: " << VK_API_VERSION_VARIANT(version)
				<< ", Major: " << VK_API_VERSION_MAJOR(version)
				<< ", Minor: " << VK_API_VERSION_MINOR(version)
				<< ", Patch: " << VK_API_VERSION_PATCH(version) << "\n";
		}

		// Set the patch of the version to 0 for best compatibility
		// patch is bits 11-0 (hence FFF)
		version &= ~(0xFFFU);

		// Alternative: Drop down to an earlier version to ensure compatibility with more devices
		// variant, major, minor, patch
		version = VK_MAKE_API_VERSION(0, 1, 0, 0);


		// Application Info
		// app name, app version, engine name, engine version, api version
		vk::ApplicationInfo appInfo = vk::ApplicationInfo(
			appName,
			version,
			"Hridiza's awesome Vulkan Engine",
			version,
			version
		);


		// GLFW Extensions
		// In Vulkan, we need to request everything explicitly
		// We need to query which extensions glfw needs to interface with Vulkan
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (debug)
		{
			extensions.push_back("VK_EXT_debug_utils");
		}

		if (debug)
		{
			std::cout << "Extensions to be requested:\n";

			for (const char* extensionName : extensions)
			{
				std::cout << "\t\"" << extensionName << "\"\n";
			}
		}


		// Layers
		std::vector<const char*> layers;
		
		if (debug)
		{
			layers.push_back("VK_LAYER_KHRONOS_validation");
		}


		// Check if requested extensions and layers are supported
		if ( !supported(extensions, layers, debug) )
		{
			return nullptr;
		}


		// Create info for instance
		// flags, app info, enabled layer count, enabled layer names,
		// enabled extension count, enabled extension names
		vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo(
			vk::InstanceCreateFlags(),
			&appInfo,
			static_cast<uint32_t>(layers.size()),
			layers.data(),
			static_cast<uint32_t>(extensions.size()),
			extensions.data()
		);

		
		// Create the instance
		// Vulkan.hpp allows us to do try/catch instead of checking = VK_SUCCESS (Vulkan.h)
		try
		{
			return vk::createInstance(createInfo);
		}
		catch (vk::SystemError err)
		{
			if (debug)
			{
				std::cout << "Failed to create instance :/\n";
			}

			return nullptr;
		}
	}
}