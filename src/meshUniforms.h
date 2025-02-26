#pragma once

#include "config.h"

namespace vkMesh
{
	vk::VertexInputBindingDescription getBasicVertexBindingDesc(uint32_t binding)
	{
		vk::VertexInputBindingDescription bindingDesc{};

		bindingDesc.binding = binding;
		bindingDesc.inputRate = vk::VertexInputRate::eVertex;
		bindingDesc.stride = sizeof(float) * ATTRIBUTE_COUNT;

		return bindingDesc;
	}

	std::array<vk::VertexInputAttributeDescription, 3> getBasicAttrDesc(uint32_t binding)
	{
		// Color
		vk::VertexInputAttributeDescription colDesc{};
		colDesc.binding = binding;
		colDesc.format = vk::Format::eR32G32B32A32Sfloat;
		colDesc.location = 0;
		colDesc.offset = 0;

		// Position
		vk::VertexInputAttributeDescription posDesc{};
		posDesc.binding = binding;
		posDesc.format = vk::Format::eR32G32B32Sfloat;
		posDesc.location = 1;
		posDesc.offset = sizeof(float) * 4;

		// UV 
		vk::VertexInputAttributeDescription uvDesc{};
		uvDesc.binding = binding;
		uvDesc.format = vk::Format::eR32G32Sfloat;
		uvDesc.location = 2;
		uvDesc.offset = sizeof(float) * (8);

		return { colDesc, posDesc, uvDesc };
	}
}
