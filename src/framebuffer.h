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
	};

	void make_framebuffers(framebufferInput inputChunk, std::vector<vkUtil::SwapchainFrame>& frames, bool debug)
	{
		for (int ii = 0; ii < frames.size(); ii++)
		{
			std::vector<vk::ImageView> attachments = { frames[ii].imageView };

			vk::FramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.flags = vk::FramebufferCreateFlags();
			framebufferInfo.renderPass = inputChunk.renderpass;
			framebufferInfo.attachmentCount = attachments.size();
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
		}
	}
}