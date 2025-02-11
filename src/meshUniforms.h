#pragma once

#include "config.h"

namespace vkMesh
{
	vk::VertexInputBindingDescription getBasicVertexBindingDesc(uint32_t binding)
	{
		vk::VertexInputBindingDescription bindingDesc{};

		bindingDesc.binding = binding;
		bindingDesc.inputRate = vk::VertexInputRate::eVertex;
		bindingDesc.stride = sizeof(float) * 7;

		return bindingDesc;
	}

	std::array<vk::VertexInputAttributeDescription, 2> getBasicAttrDesc(uint32_t binding)
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

		return { colDesc, posDesc };
	}
}
