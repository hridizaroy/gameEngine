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
		const DescriptorSetLayoutData& bindings)
	{
		std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
		layoutBindings.reserve(bindings.count);

		for (int ii = 0; ii < bindings.count; ii++)
		{
			vk::DescriptorSetLayoutBinding layoutBinding;
			layoutBinding.binding = bindings.indices[ii];
			layoutBinding.descriptorCount = static_cast<uint32_t>(bindings.counts[ii]);
			layoutBinding.descriptorType = bindings.types[ii];
			layoutBinding.stageFlags = bindings.stages[ii];

			layoutBindings.push_back(layoutBinding);
		}

		vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.flags = vk::DescriptorSetLayoutCreateFlagBits();
		layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.count);
		layoutCreateInfo.pBindings = layoutBindings.data();

		try
		{
			return device.createDescriptorSetLayout(layoutCreateInfo);
		}
		catch (const vk::SystemError&)
		{
			return nullptr;
		}
	}

	vk::DescriptorPool make_descriptor_pool(const vk::Device& logicalDevice,
		uint32_t size, const DescriptorSetLayoutData& bindings)
	{
		std::vector<vk::DescriptorPoolSize> poolSizes;
		poolSizes.reserve(bindings.count);

		for (int ii = 0; ii < bindings.count; ii++)
		{
			vk::DescriptorPoolSize poolSize;
			poolSize.type = bindings.types[ii];
			poolSize.descriptorCount = size;

			poolSizes.push_back(poolSize);
		}

		vk::DescriptorPoolCreateInfo poolCreateInfo;
		poolCreateInfo.flags = vk::DescriptorPoolCreateFlags();
		poolCreateInfo.maxSets = size;
		poolCreateInfo.poolSizeCount = static_cast<uint32_t>(bindings.count);
		poolCreateInfo.pPoolSizes = poolSizes.data();

		try
		{
			return logicalDevice.createDescriptorPool(poolCreateInfo);
		}
		catch (const vk::SystemError&)
		{
			return nullptr;
		}
	}

	vk::DescriptorSet allocate_descriptor_set(
		const vk::Device& logicalDevice,
		const vk::DescriptorPool& descriptorPool,
		const vk::DescriptorSetLayout& layout
	)
	{
		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		try
		{
			return logicalDevice.allocateDescriptorSets(allocInfo)[0];
		}
		catch (const vk::SystemError&)
		{
			return nullptr;
		}
	}
}