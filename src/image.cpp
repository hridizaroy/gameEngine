#include "config.h"

#include "logging.h"
#include "descriptors.h"
#include "buffers.h"

#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "singleTimeCommands.h"

vkImage::Texture::Texture(TextureInputChunk input):
	logicalDevice(input.logicalDevice),
	physicalDevice(input.physicalDevice),
	filename(input.filename),
	commandBuffer(input.commandBuffer),
	queue(input.queue),
	layout(input.layout),
	descriptorPool(input.descriptorPool)
{
	load();

	ImageInputChunk imageInput;
	imageInput.logicalDevice = logicalDevice;
	imageInput.physicalDevice = physicalDevice;
	imageInput.height = height;
	imageInput.width = width;
	// Image is laid out in memory in whatever way the device considers optimal
	imageInput.tiling = vk::ImageTiling::eOptimal;
	// We are going to copy stuff into this image, and also sample it
	imageInput.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	imageInput.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal; // High performance memory

	image = make_image(imageInput);
	imageMemory = make_image_memory(imageInput, image);

	populate();

	free(pixels); // TODO: RAII VIOLATION?

	make_view();
	make_sampler();
	make_descriptor_set();
}
 
vkImage::Texture::~Texture()
{
	logicalDevice.freeMemory(imageMemory);
	logicalDevice.destroyImage(image);
	logicalDevice.destroyImageView(imageView);
	logicalDevice.destroySampler(sampler);
}

void vkImage::Texture::use(vk::CommandBuffer& commandBuffer,
	const vk::PipelineLayout& pipelineLayout)
{
	// TODO: Don't hardcode '1'
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
		pipelineLayout, 1, descriptorSet, nullptr);
}

void vkImage::Texture::load()
{
	pixels = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
	{
		// TODO: Use a logging function
		std::cout << "Failed to load " << filename << "\n";
	}
}

void vkImage::Texture::populate()
{
	vkUtil::BufferInput input;
	input.logicalDevice = logicalDevice;
	input.physicalDevice = physicalDevice;
	input.memoryProperties = vk::MemoryPropertyFlagBits::eHostCoherent |
								vk::MemoryPropertyFlagBits::eHostVisible;
	input.usage = vk::BufferUsageFlagBits::eTransferSrc;
	input.size = width * height * 4;

	vkUtil::BufferData stagingBuffer = vkUtil::create_buffer(input);

	void* writeLocation = logicalDevice.mapMemory(stagingBuffer.bufferMemory, 0, input.size);
	memcpy(writeLocation, pixels, input.size);
	logicalDevice.unmapMemory(stagingBuffer.bufferMemory);

	ImageLayoutTransitionJob transitionJob;
	transitionJob.commandBuffer = commandBuffer;
	transitionJob.queue = queue;
	transitionJob.image = image;
	transitionJob.oldLayout = vk::ImageLayout::eUndefined;
	transitionJob.newLayout = vk::ImageLayout::eTransferDstOptimal;
	transition_image_layout(transitionJob);

	BufferImageCopyJob copyJob;
	copyJob.commandBuffer = commandBuffer;
	copyJob.queue = queue;
	copyJob.srcBuffer = stagingBuffer.buffer;
	copyJob.dstImage = image;
	copyJob.width = width;
	copyJob.height = height;
	copy_buffer_to_image(copyJob);

	transitionJob.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	transitionJob.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	transition_image_layout(transitionJob);

	logicalDevice.freeMemory(stagingBuffer.bufferMemory);
	logicalDevice.destroyBuffer(stagingBuffer.buffer);
}

void vkImage::Texture::make_view()
{
	imageView = make_image_view(logicalDevice, image, vk::Format::eR8G8B8A8Unorm);
}

void vkImage::Texture::make_sampler()
{
	vk::SamplerCreateInfo samplerInfo;
	samplerInfo.flags = vk::SamplerCreateFlags();
	samplerInfo.minFilter = vk::Filter::eNearest;
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	samplerInfo.anisotropyEnable = false;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerInfo.unnormalizedCoordinates = false;
	samplerInfo.compareOp = vk::CompareOp::eAlways;
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	try
	{
		sampler = logicalDevice.createSampler(samplerInfo);
	}
	catch (const vk::SystemError& e)
	{
		std::cout << "Failed to make sampler\n";
	}
}

void vkImage::Texture::make_descriptor_set()
{
	descriptorSet = vkInit::allocate_descriptor_set(logicalDevice, descriptorPool, layout);

	vk::DescriptorImageInfo imageDescriptor;
	imageDescriptor.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	imageDescriptor.imageView = imageView;
	imageDescriptor.sampler = sampler;

	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageDescriptor;

	logicalDevice.updateDescriptorSets(descriptorWrite, nullptr);
}

vk::Image vkImage::make_image(ImageInputChunk input)
{
	vk::ImageCreateInfo imageInfo;
	imageInfo.flags = vk::ImageCreateFlagBits();
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent = vk::Extent3D(input.width, input.height, 1);
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = vk::Format::eR8G8B8A8Unorm;
	imageInfo.tiling = input.tiling;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageInfo.usage = input.usage;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.samples = vk::SampleCountFlagBits::e1;

	try
	{
		return input.logicalDevice.createImage(imageInfo);
	}
	catch (const vk::SystemError& err)
	{
		std::cout << "Unable to make image\n";
	}
}

vk::DeviceMemory vkImage::make_image_memory(ImageInputChunk input, vk::Image image)
{
	vk::MemoryRequirements requirements = input.logicalDevice.getImageMemoryRequirements(image);

	vk::MemoryAllocateInfo allocation;
	allocation.allocationSize = requirements.size;
	allocation.memoryTypeIndex = vkUtil::find_memory_type_idx(
									input.physicalDevice,
									requirements.memoryTypeBits,
									input.memoryProperties);
	
	try
	{
		vk::DeviceMemory imageMemory = input.logicalDevice.allocateMemory(allocation);
		input.logicalDevice.bindImageMemory(image, imageMemory, 0);
		return imageMemory;
	}
	catch (const vk::SystemError& err)
	{
		std::cout << "Unable to allocate memory for image\n";
	}
}

void vkImage::transition_image_layout(ImageLayoutTransitionJob job)
{
	vkUtil::start_job(job.commandBuffer);

	vk::ImageSubresourceRange access;
	access.aspectMask = vk::ImageAspectFlagBits::eColor;
	access.baseMipLevel = 0;
	access.levelCount = 1;
	access.baseArrayLayer = 0;
	access.layerCount = 1;

	vk::ImageMemoryBarrier barrier;
	barrier.oldLayout = job.oldLayout;
	barrier.newLayout = job.newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = job.image;
	barrier.subresourceRange = access;

	vk::PipelineStageFlagBits sourceStage, dstStage;
	if (job.oldLayout == vk::ImageLayout::eUndefined)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		
		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader;
	}

	job.commandBuffer.pipelineBarrier(sourceStage, dstStage, vk::DependencyFlags(), nullptr, nullptr, barrier);

	vkUtil::end_job(job.commandBuffer, job.queue);
}

void vkImage::copy_buffer_to_image(BufferImageCopyJob job)
{
	vkUtil::start_job(job.commandBuffer);

	vk::BufferImageCopy copy;
	copy.bufferOffset = 0;
	copy.bufferRowLength = 0;
	copy.bufferImageHeight = 0;

	vk::ImageSubresourceLayers access;
	access.aspectMask = vk::ImageAspectFlagBits::eColor;
	access.mipLevel = 0;
	access.baseArrayLayer = 0;
	access.layerCount = 1;
	copy.imageSubresource = access;

	copy.imageOffset = vk::Offset3D(0, 0, 0);
	copy.imageExtent = vk::Extent3D(job.width, job.height, 1);

	job.commandBuffer.copyBufferToImage(job.srcBuffer, job.dstImage, vk::ImageLayout::eTransferDstOptimal, copy);

	vkUtil::end_job(job.commandBuffer, job.queue);
}

vk::ImageView vkImage::make_image_view(vk::Device logicalDevice, vk::Image image, vk::Format format)
{
	vk::ImageViewCreateInfo createInfo = {};

	createInfo.image = image;
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

	createInfo.format = format;

	return logicalDevice.createImageView(createInfo);
}
