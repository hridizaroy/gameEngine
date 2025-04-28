#pragma once

#include "config.h"
#include "Entity.h"
#include "buffers.h"


struct FinalizationChunk
{
	const vk::Device& logicalDevice;
	const vk::PhysicalDevice& physicalDevice;
	const vk::Queue& queue;
	const vk::CommandBuffer& commandBuffer;
};

class Scene
{
public:
	Scene();
	~Scene();

	//std::vector<glm::vec3> trianglePositions;
	//std::vector<glm::vec3> pentagonPositions;
	//std::vector<glm::vec3> hexagonPositions;


	void InitEntities(); 
	std::vector<REntity> entities; 




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


};




class SceneData
{

};