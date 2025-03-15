#pragma once

#include "config.h"
#include "Entity.h"
#include "buffers.h"

// TODO: Organize into namespace 


struct FinalizationChunk
{
	const vk::Device& logicalDevice;
	const vk::PhysicalDevice& physicalDevice;
	const vk::Queue& queue;
	const vk::CommandBuffer& commandBuffer;
};



//const uint32_t GetShapeParamCount(uint32_t shapeType);


class Scene
{
public:
	Scene();
	~Scene();

	void InitEntities(); 
	std::vector<REntity*> rasterEntities; 
	std::vector<SEntity*> shapeEntities; 

public:

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

	// We need to keep count of the offset to the most current shape's parameters 
	uint32_t paramEnd; 
	void AddShapeEntity(uint32_t shapeType, std::string nameID, std::vector<float> parameters);

};