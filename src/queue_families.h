#pragma once

#include "config.h"

namespace vkUtil
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};


	QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface, bool debug)
	{
		QueueFamilyIndices indices;

		// Get queue families for device
		std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

		if (debug)
		{
			std::cout << "System can support " << queueFamilies.size() << " queue families.\n";
		}


		// Go through each device queue family and add indices accordingly
		int idx = 0;
		for (const vk::QueueFamilyProperties& queueFamily : queueFamilies)
		{
			// check if this is a graphics queue family
			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				indices.graphicsFamily = idx;

				if (debug)
				{
					std::cout << "Queue Family " << idx << " is suitable for graphics.\n";
				}
			}

			if (device.getSurfaceSupportKHR(idx, surface))
			{
				indices.presentFamily = idx;

				if (debug)
				{
					std::cout << "Queue Family " << idx << " is suitable for presenting.\n";
				}
			}


			// if we found all needed queue family indices, break
			if (indices.isComplete())
			{
				break;
			}

			idx++;
		}


		return indices;
	}
}