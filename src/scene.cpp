#include "scene.h"

Scene::Scene()
{
	offset = 0;
}


/// <summary>
/// Generates the entities that will be rendered in our scene
/// </summary>
void Scene::InitEntities()
{

	// TODO: Load from file here 

	//for (int y = -10; y < 10; y += 2)
	//{
	//	REntity entity;
	//
	//	std::shared_ptr<UInfo> info = std::make_shared<UInfo>();
	//	info->name = "ID: " + std::to_string(y);
	//	info->transform = std::make_shared<Transform>();
	//	info->transform->MoveAbs(glm::vec3(-0.4f, y * 0.1f, 0.0f));
	//
	//	entity.info = info;
	//	entity.meshType = TRIANGLE;
	//	
	//	entities.push_back(entity);
	//}

	
	REntity* entity = new REntity;
	
	std::shared_ptr<UInfo> info = std::make_shared<UInfo>();
	info->name = "ID: Fullscreen";
	info->transform = std::make_shared<Transform>();
	
	entity->info = info;
	entity->meshType = TRIANGLE_FULLSCREEN;
	
	entities.push_back(entity);
	
}







void Scene::consume(const MeshType& meshType, const std::vector<float>& vertexData)
{
	lump.reserve(lump.size() + vertexData.size());

	for (const float& attribute : vertexData)
	{
		lump.push_back(attribute);
	}

	size_t vertexCount = vertexData.size() / ATTRIBUTE_COUNT;

	offsets_sizes.insert(
		std::make_pair(
			meshType,
			std::pair(offset, vertexCount)
		)
	);

	offset += vertexCount;
}

vk::Buffer Scene::getVertexBuffer() const
{
	return vertexBufferData.buffer;
}

std::pair<size_t, size_t> Scene::lookupOffsetSize(const MeshType& meshType)
{
	auto it = offsets_sizes.find(meshType);

	if (it != offsets_sizes.end())
	{
		return it->second;
	}

	return { 0, 0 };
}


void Scene::finalize(const FinalizationChunk& finalizationChunk)
{
	vkUtil::BufferInput inputChunk{};
	inputChunk.logicalDevice = finalizationChunk.logicalDevice;
	inputChunk.physicalDevice = finalizationChunk.physicalDevice;
	inputChunk.usage = vk::BufferUsageFlagBits::eTransferSrc;
	inputChunk.size = sizeof(float) * lump.size();
	// Host visible = we can write to it directly
	// Host coherent = Write operation happens right on the location,
	// we don't have to worry about sync
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible |
		vk::MemoryPropertyFlagBits::eHostCoherent;

	// Copy to temp location in GPU
	vkUtil::BufferData tempBufferData = vkUtil::create_buffer(inputChunk);

	void* memoryLocation = finalizationChunk.logicalDevice.mapMemory(
		tempBufferData.bufferMemory, 0, inputChunk.size);

	memcpy(memoryLocation, lump.data(), inputChunk.size);

	finalizationChunk.logicalDevice.unmapMemory(tempBufferData.bufferMemory);

	// Copy from temp GPU location to high performance area
	inputChunk.usage = vk::BufferUsageFlagBits::eTransferDst
		| vk::BufferUsageFlagBits::eVertexBuffer;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

	vertexBufferData = vkUtil::create_buffer(inputChunk);

	vkUtil::copy_buffer(tempBufferData, vertexBufferData,
		inputChunk.size, finalizationChunk.queue,
		finalizationChunk.commandBuffer);

	// free temp buffer
	finalizationChunk.logicalDevice.destroyBuffer(tempBufferData.buffer);
	finalizationChunk.logicalDevice.freeMemory(tempBufferData.bufferMemory);
}


void Scene::cleanup(const vk::Device& logicalDevice) const
{
	logicalDevice.destroyBuffer(vertexBufferData.buffer);
	logicalDevice.freeMemory(vertexBufferData.bufferMemory);

	delete this;
}

Scene::~Scene()
{
	for (auto entity : entities)
	{
		delete entity;
	}
	entities.clear();
}