#pragma once

#include "config.h"
#include "logging.h"
#include "queue_families.h"

namespace vkInit
{

	bool checkDeviceExtensionSupport(
		const vk::PhysicalDevice& device,
		const std::vector<const char*>& requestedExtensions,
		const bool& debug
	)
	{
		// Create a set to get unique values of requested extensions
		std::set<std::string> requestedExtensionsSet(requestedExtensions.begin(), requestedExtensions.end());

		if (debug)
		{
			std::cout << "Device can support extensions:\n";
		}


		// Check which extensions the device can support
		for (vk::ExtensionProperties& extension : device.enumerateDeviceExtensionProperties())
		{
			if (debug)
			{
				std::cout << "\t\"" << extension.extensionName << "\"\n";
			}

			// "Check off" this extension from the requested extensions list
			requestedExtensionsSet.erase(extension.extensionName);
		}

		// if all requested extensions were found, this set would be empty
		return requestedExtensionsSet.empty();
	}


	bool isSuitable(const vk::PhysicalDevice& device, const bool debug)
	{
		if (debug)
		{
			std::cout << "Checking if device is suitable...\n";
		}

		// For now, we consider a device suitable if it can present to the screen
		// i.e., Support the swapchain extension
		const std::vector<const char*> requestedExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		if (debug)
		{
			std::cout << "We are requesting device extensions:\n";

			for (const char *extension : requestedExtensions)
			{
				std::cout << "\t\"" << extension << "\"\n";
			}
		}


		// Check if device can support our requested extensions
		if (checkDeviceExtensionSupport(device, requestedExtensions, debug))
		{
			if (debug)
			{
				std::cout << "Device can support the requested extensions!\n";
			}
		}
		else
		{
			if (debug)
			{
				std::cout << "Device cannot support the requested extensions!\n";
			}

			return false;
		}

		return true;
	}


	vk::PhysicalDevice choose_physical_device(vk::Instance& instance, bool debug)
	{
		// Physical devices are neither created nor destroyed. Merely chosen.
		
		if (debug)
		{
			std::cout << "Choosing Physical Device...\n";
		}


		// Get available devices
		std::vector<vk::PhysicalDevice> availableDevices = instance.enumeratePhysicalDevices();
		
		if (debug)
		{
			std::cout << "There are " << availableDevices.size() << " physical device(s) available on this system\n";
		}

		vk::PhysicalDevice chosenDevice = nullptr;

		// TODO: create a priority list by device type and choose accordingly
		// Check if any device is suitable
		for (vk::PhysicalDevice device : availableDevices)
		{
			if (debug)
			{
				log_device_properties(device);
			}

			if (isSuitable(device, debug))
			{
				vk::PhysicalDeviceProperties properties = device.getProperties();
				chosenDevice = device;

				// Prioritize Discrete GPUs
				if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
				{
					break;
				}
			}
		}

		return chosenDevice;
	}


	vk::Device create_logical_device(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool debug)
	{
		vkUtil::QueueFamilyIndices indices = vkUtil::findQueueFamilies(physicalDevice, surface, debug);

		// Get unique indices for queue families
		std::vector<uint32_t> uniqueIndices;
		uniqueIndices.push_back(indices.graphicsFamily.value());

		if (indices.graphicsFamily.value() != indices.presentFamily.value())
		{
			uniqueIndices.push_back(indices.presentFamily.value());
		}

		// Queue priority determines how GPU allocates its resources towards different queues
		// in the same queue family
		// 0.0 = lowest, 1.0 = highest
		float queuePriority = 1.0f;

		// Queue info
		// flags, queue family index, queue count, pQueuePriorities
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfo;

		for (uint32_t queueFamilyIdx : uniqueIndices)
		{
			queueCreateInfo.push_back(
				vk::DeviceQueueCreateInfo(
					vk::DeviceQueueCreateFlags(),
					queueFamilyIdx,
					1,
					&queuePriority
				)
			);
		}

		// Request swapchain extension
		std::vector<const char*> deviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};



		// Device features
		// We can enable features in this if we want
		// e.g., deviceFeatures.samplerAnisotropy = true
		vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();


		// Enabled layers
		std::vector<const char*> enabledLayers;

		if (debug)
		{
			enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
		}


		// Device Create info
		// flags, queueCreateInfoCount, pQueueCreateInfos, enabledLayerCount, enabled layers,
		// enabledExtensionCount, enabled extensions, pEnabledFeatures
		vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
			vk::DeviceCreateFlags(),
			queueCreateInfo.size(), queueCreateInfo.data(),
			enabledLayers.size(),
			enabledLayers.data(),
			deviceExtensions.size(), deviceExtensions.data(),
			&deviceFeatures
		);


		// Create the device
		try
		{
			vk::Device device = physicalDevice.createDevice(deviceInfo);

			if (debug)
			{
				std::cout << "Logical device created!\n";
			}
			
			return device;
		}
		catch (vk::SystemError err)
		{
			if (debug)
			{
				std::cout << "Logical device creation failed.\n";

				return nullptr;
			}
		}

		return nullptr;
	}


	std::array<vk::Queue, 2> get_queue(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface, bool debug)
	{
		vkUtil::QueueFamilyIndices indices = vkUtil::findQueueFamilies(physicalDevice, surface, debug);

		// queue family index, queue index
		return
		{
			device.getQueue(indices.graphicsFamily.value(), 0),
			device.getQueue(indices.presentFamily.value(), 0)
		};
	}
}