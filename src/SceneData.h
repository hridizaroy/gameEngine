#pragma once

#include "config.h"

#include "buffers.h"

struct FinalizationChunk
{
	const vk::Device& logicalDevice;
	const vk::PhysicalDevice& physicalDevice;
	const vk::Queue& queue;
	const vk::CommandBuffer& commandBuffer;
};

class SceneData
{
public:
	SceneData();
	~SceneData();

	void consume(const MeshType& mesh, const std::vector<float>& vertexData);
	void finalize(const FinalizationChunk& finalizationChunk);

	vk::Buffer getVertexBuffer() const;

	std::pair<size_t, size_t> lookupOffsetSize(const MeshType& meshType);

	void cleanup(const vk::Device& logicalDevice) const;

private:
	vkUtil::BufferData vertexBufferData;
	std::unordered_map<MeshType, std::pair<size_t, size_t>> offsets_sizes;	
	size_t offset;

	std::vector<float> lump;
};