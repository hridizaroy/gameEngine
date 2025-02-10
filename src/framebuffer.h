#pragma once

#include "config.h"
#include "frame.h"


namespace vkInit
{
	struct framebufferInput
	{
		vk::Device device;
		vk::RenderPass renderpass;
		vk::Extent2D swapchainExtent;

		// imgui
		vk::RenderPass imguiRenderpass;
	};

	void make_framebuffers(framebufferInput inputChunk, std::vector<vkUtil::SwapchainFrame>& frames, bool debug)
	{
		for (int ii = 0; ii < frames.size(); ii++)
		{
			// TODO: Make this an array instead for efficiency?
			std::vector<vk::ImageView> attachments = { frames[ii].imageView };

			vk::FramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.flags = vk::FramebufferCreateFlags();
			framebufferInfo.renderPass = inputChunk.renderpass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = inputChunk.swapchainExtent.width;
			framebufferInfo.height = inputChunk.swapchainExtent.height;
			framebufferInfo.layers = 1;

			try
			{
				frames[ii].frameBuffer = inputChunk.device.createFramebuffer(framebufferInfo);

				if (debug)
				{
					std::cout << "Created framebuffer for frame " << ii << std::endl;
				}
			}
			catch (vk::SystemError err)
			{
				if (debug)
				{
					std::cout << "Failed to create framebuffer for frame " << ii << std::endl;
				}
			}


			// imgui framebuffer
			{
				vk::ImageView attachment[1];

				// TODO: is this correct?
				attachment[0] = frames[ii].imageView;

				vk::FramebufferCreateInfo info {};
				info.renderPass = inputChunk.imguiRenderpass;
				info.attachmentCount = 1;
				info.pAttachments = attachment;
				info.width = inputChunk.swapchainExtent.width;
				info.height = inputChunk.swapchainExtent.height;
				info.layers = 1;

				try
				{
					frames[ii].imguiFrameBuffer = inputChunk.device.createFramebuffer(info);

					if (debug)
					{
						std::cout << "Created imgui framebuffer for frame " << ii << std::endl;
					}
				}
				catch (vk::SystemError err)
				{
					if (debug)
					{
						std::cout << "Failed to create imgui framebuffer for frame " << ii << std::endl;
					}
				}
			}

		}
	}
}