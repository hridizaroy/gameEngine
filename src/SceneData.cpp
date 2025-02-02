#include "SceneData.h"

SceneData::SceneData()
{
	offset = 0;
}

void SceneData::consume(const MeshType& meshType, const std::vector<float>& vertexData)
{
	lump.reserve(lump.size() + vertexData.size());

	for (const float& attribute : vertexData)
	{
		lump.push_back(attribute);
	}

	size_t vertexCount = vertexData.size() / 7;

	offsets_sizes.insert(
		std::make_pair(
			meshType,
			std::pair(offset, vertexCount)
		)
	);

	offset += vertexCount;
}

vk::Buffer SceneData::getVertexBuffer() const
{
	return vertexBufferData.buffer;
}

std::pair<size_t, size_t> SceneData::lookupOffsetSize(const MeshType& meshType)
{
	auto it = offsets_sizes.find(meshType);

	if (it != offsets_sizes.end())
	{
		return it->second;
	}

	return { 0, 0 };
}


void SceneData::finalize(const vk::PhysicalDevice& physicalDevice, const vk::Device& logicalDevice)
{
	vkUtil::BufferInput inputChunk{};
	inputChunk.logicalDevice = logicalDevice;
	inputChunk.physicalDevice = physicalDevice;
	inputChunk.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	inputChunk.size = sizeof(float) * lump.size();

	vertexBufferData = vkUtil::create_buffer(inputChunk);

	void* memoryLocation = logicalDevice.mapMemory(vertexBufferData.bufferMemory, 0, inputChunk.size);
	memcpy(memoryLocation, lump.data(), inputChunk.size);
	logicalDevice.unmapMemory(vertexBufferData.bufferMemory);
}


void SceneData::cleanup(const vk::Device& logicalDevice) const
{
	logicalDevice.destroyBuffer(vertexBufferData.buffer);
	logicalDevice.freeMemory(vertexBufferData.bufferMemory);

	delete this;
}

SceneData::~SceneData()
{

}