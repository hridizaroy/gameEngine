#pragma once

#include "config.h"
#include "logging.h"
#include "queue_families.h"
#include "frame.h"


namespace vkInit
{
	struct SwapchainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	struct SwapchainBundle
	{
		vk::SwapchainKHR swapchain;
		std::vector<vkUtil::SwapchainFrame> frames;
		vk::Format format;
		vk::Extent2D extent;
	};


	SwapchainSupportDetails query_swapchain_support(vk::PhysicalDevice device, vk::SurfaceKHR surface, bool debug)
	{
		SwapchainSupportDetails support;

		// Capabilities
		support.capabilities = device.getSurfaceCapabilitiesKHR(surface);

		if (debug)
		{
			std::cout << "Swapchain can support the following surface capabilities:\n";

			std::cout << "\tMinimum image count: " << support.capabilities.minImageCount << "\n";
			std::cout << "\tMaximum image count: " << support.capabilities.maxImageCount << "\n";

			std::cout << "\tCurrent extent:\n";

			std::cout << "\t\tWidth: " << support.capabilities.currentExtent.width << "\n";
			std::cout << "\t\tHeight: " << support.capabilities.currentExtent.height << "\n";

			std::cout << "\tMinimum supported extent:\n";
			std::cout << "\t\tWidth: " << support.capabilities.minImageExtent.width << "\n";
			std::cout << "\t\tHeight: " << support.capabilities.minImageExtent.height << "\n";

			std::cout << "\tMaximum supported extent:\n";
			std::cout << "\t\tWidth: " << support.capabilities.maxImageExtent.width << "\n";
			std::cout << "\t\tHeight: " << support.capabilities.maxImageExtent.height << "\n";

			std::cout << "\tMaximum image array layers: " << support.capabilities.maxImageArrayLayers << "\n";

			std::cout << "\tSupported transforms:\n";
			std::vector<std::string> stringList = log_transform_bits(support.capabilities.supportedTransforms);
			for (std::string line : stringList)
			{
				std::cout << "\t\t" << line << "\n";
			}

			std::cout << "\tCurrent transforms:\n";
			stringList = log_transform_bits(support.capabilities.currentTransform);
			for (std::string line : stringList)
			{
				std::cout << "\t\t" << line << "\n";
			}

			std::cout << "\tSupported alpha operations:\n";
			stringList = log_alpha_composite_bits(support.capabilities.supportedCompositeAlpha);
			for (std::string line : stringList)
			{
				std::cout << "\t\t" << line << "\n";
			}

			std::cout << "\tSupported image usage:\n";
			stringList = log_image_usage_bits(support.capabilities.supportedUsageFlags);
			for (std::string line : stringList)
			{
				std::cout << "\t\t" << line << "\n";
			}
		}


		// Formats
		support.formats = device.getSurfaceFormatsKHR(surface);

		if (debug)
		{
			for (vk::SurfaceFormatKHR supportedFormat : support.formats)
			{
				std::cout << "Supported pixel format: " << vk::to_string(supportedFormat.format) << "\n";
				std::cout << "Supported color space: " << vk::to_string(supportedFormat.colorSpace) << "\n";
			}
		}


		// Present modes
		support.presentModes = device.getSurfacePresentModesKHR(surface);

		for (vk::PresentModeKHR presentMode : support.presentModes)
		{
			std::cout << "\t" << log_present_mode(presentMode) << "\n";
		}


		return support;
	}


	vk::SurfaceFormatKHR choose_swapchain_surface_format(std::vector<vk::SurfaceFormatKHR> formats)
	{
		// Check if our preferred format is available
		for (vk::SurfaceFormatKHR format : formats)
		{
			if (format.format == vk::Format::eB8G8R8A8Unorm
				&& format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return format;
			}
		}

		// Otherwise, just return any format
		return formats[0];
	}


	vk::PresentModeKHR choose_swapchain_present_mode(std::vector<vk::PresentModeKHR> presentModes)
	{
		// Check if our preferred presentMode is available
		for (vk::PresentModeKHR presentMode : presentModes)
		{
			if (presentMode == vk::PresentModeKHR::eMailbox)
			{
				return presentMode;
			}
		}

		// Otherwise, return fifo since it's guaranteed to exist
		return vk::PresentModeKHR::eFifo;
	}


	vk::Extent2D choose_swapchain_extent(uint32_t width, uint32_t height, vk::SurfaceCapabilitiesKHR capabilities)
	{
		// UINT32_MAX => You're allowed to have the image extent differ from the window extent
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			vk::Extent2D extent = { width, height };

			// if image extent < min extent, choose min extent
			// if image extent > max extent, choose max extent

			extent.width = std::min(
				capabilities.maxImageExtent.width,
				std::max(capabilities.minImageExtent.width, width)
			);

			extent.height = std::min(
				capabilities.maxImageExtent.height,
				std::max(capabilities.minImageExtent.height, height)
			);
		}
	}


	SwapchainBundle create_swapchain(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, int width, int height, bool debug)
	{
		if (debug)
		{
			std::cout << "Creating Swapchain...\n";
		}

		SwapchainSupportDetails support = query_swapchain_support(physicalDevice, surface, debug);

		vk::SurfaceFormatKHR format = choose_swapchain_surface_format(support.formats);

		vk::PresentModeKHR presentMode = choose_swapchain_present_mode(support.presentModes);

		vk::Extent2D extent = choose_swapchain_extent(width, height, support.capabilities);

		// Increase frame rate by requesting 1 additional image
		uint32_t imageCount = std::min(
			support.capabilities.maxImageCount,
			support.capabilities.minImageCount + 1
		);


		// flags, surface, minImageCount, imageFormat, imageColorSpace, imageExtent
		// imageArrayLayers, imageUsage
		vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
			vk::SwapchainCreateFlagsKHR(), surface, imageCount, format.format,
			format.colorSpace, extent, 1, vk::ImageUsageFlagBits::eColorAttachment
		);

		vkUtil::QueueFamilyIndices indices = vkUtil::findQueueFamilies(physicalDevice, surface, debug);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (queueFamilyIndices[0] != queueFamilyIndices[1])
		{
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		}

		createInfo.preTransform = support.capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);


		SwapchainBundle bundle{};

		try
		{
			bundle.swapchain = logicalDevice.createSwapchainKHR(createInfo);

			if (debug)
			{
				std::cout << "Successfully created swapchain!\n";
			}
		}
		catch (vk::SystemError err)
		{
			throw std::runtime_error("Failed to create swapchain :/\n");
		}


		// Create imageviews for the swapchain
		std::vector<vk::Image> images = logicalDevice.getSwapchainImagesKHR(bundle.swapchain);
		
		bundle.frames.resize(images.size());

		for (size_t ii = 0; ii < images.size(); ii++)
		{
			vk::ImageViewCreateInfo createInfo = {};

			createInfo.image = images[ii];
			createInfo.viewType = vk::ImageViewType::e2D;

			createInfo.components.r = vk::ComponentSwizzle::eIdentity;
			createInfo.components.g = vk::ComponentSwizzle::eIdentity;
			createInfo.components.b = vk::ComponentSwizzle::eIdentity;
			createInfo.components.a = vk::ComponentSwizzle::eIdentity;

			createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;

			// no mipmapping
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;

			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			createInfo.format = format.format;


			bundle.frames[ii].image = images[ii];
			bundle.frames[ii].imageView = logicalDevice.createImageView(createInfo);
		}

		bundle.format = format.format;
		bundle.extent = extent;


		return bundle;
	}
}