#pragma once

#include "config.h"

namespace vkInit
{
	struct DescriptorSetLayoutData
	{
		size_t count;
		std::vector<uint32_t> indices;
		std::vector<vk::DescriptorType> types;
		std::vector<size_t> counts;
		std::vector<vk::ShaderStageFlags> stages;
	};

	vk::DescriptorSetLayout make_descriptor_set_layout(const vk::Device& device,
		const DescriptorSetLayoutData& bindings);

	vk::DescriptorPool make_descriptor_pool(const vk::Device& logicalDevice,
		uint32_t size, const DescriptorSetLayoutData& bindings);

	vk::DescriptorSet allocate_descriptor_set(
		const vk::Device& logicalDevice,
		const vk::DescriptorPool& descriptorPool,
		const vk::DescriptorSetLayout& layout
	);
}