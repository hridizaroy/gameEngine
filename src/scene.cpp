#include "scene.h"

Scene::Scene()
{
	offset = 0;
	paramEnd = 0;

	InitEntities();



}


/// <summary>
/// Generates the entities that will be rendered in our scene
/// </summary>
void Scene::InitEntities()
{

	// TODO: Load from file here 

	REntity* entity = new REntity;

	std::shared_ptr<UInfo> info = std::make_shared<UInfo>(); 
	info->name = "ID: Fullscreen"; 
	info->transform = std::make_shared<Transform>();

	entity->info = info;
	entity->meshType = TRIANGLE_FULLSCREEN;

	rasterEntities.push_back(entity);

	// TODO: 
	// [ ] Ensure that ID's are not repeated 
	// [ ] Ensure parameter length matches with shape 
	// [ ] Read shape data from file 

	std::vector<float> tBox = std::vector<float>(
		{
		 0.5f,  0.0f, -1.0f,	// Center
		 0.1f,  0.1f,  0.1f,	// Size
		 0.005f,				// Thickness 
		}
	);
	AddShapeEntity(FRAME_BOX, "TestingBox", tBox);


	std::vector<float> tSphere = std::vector<float>(
		{
		-0.5f, 0.0f, -1.0f,		// Center 
		 0.1f					// Radius 
		}
	);
	AddShapeEntity(SPHERE, "TestingSphere", tSphere);
}

/// <summary>
/// Adding a shape entity requires us to keep track of its individual shape 
/// param data in a single array. 
/// </summary>
/// <param name="shapeType"></param>
/// <param name="nameID"></param>
void Scene::AddShapeEntity(uint32_t shapeType, std::string nameID, std::vector<float> parameters)
{
	SEntity* entity = new SEntity();
	entity->info = std::make_shared<UInfo>();
	entity->info->name = nameID; 
	entity->info->transform = std::make_shared<Transform>();
	
	// Shape holds a vec4 of the data. First entry is shape type and
	// second is the param end. We store the transform infomation in
	// the info section 
	entity->shape = Shape({ glm::uvec4(shapeType, paramEnd, 0, 0) });
	uint32_t paramCount = ShapeTypes::GetShapeParamSize(shapeType);
	paramEnd += paramCount;

	// Adds parameter to be managed here 

	// TODO: Compress floats into groups of vec4s instead
	//		 of having a vec4 per parameters!!!! 


	glm::vec4* paramsInVec4s = new glm::vec4[paramCount];

	for (int i = 0; i < paramCount; i++)
	{
		// Convert parameter in a vec4 for padding 
		paramsInVec4s[i] = {parameters[i], 0, 0, 0};
	}

	entity->parameteres = paramsInVec4s;

	shapeEntities.push_back(entity);
}

/// <summary>
/// Adds a entity using a 
/// </summary>
void Scene::AddShapeEntity(uint32_t shapeType, std::string nameID, glm::vec4* parameters)
{







	SEntity* entity = new SEntity();
	entity->info = std::make_shared<UInfo>();
	entity->info->name = nameID;
	entity->info->transform = std::make_shared<Transform>();

	// Shape holds a vec4 of the data. First entry is shape type and
	// second is the param end. We store the transform infomation in
	// the info section 
	entity->shape = Shape({ glm::uvec4(shapeType, paramEnd, 0, 0) });
	uint32_t paramCount = ShapeTypes::GetShapeParamSize(shapeType);
	paramEnd += paramCount;

	// Adds parameter to be managed here 

	// TODO: Compress floats into groups of vec4s instead
	//		 of having a vec4 per parameters!!!! 



	// Duplicate data into a seperate array  
	uint32_t shapeLength = ShapeTypes::GetShapeParamSize(shapeType);
	glm::vec4* parametersDupe= new glm::vec4[shapeLength];
	memcpy(parametersDupe, parameters, (size_t)shapeLength * sizeof(glm::vec4));


	entity->parameteres = parametersDupe;

	shapeEntities.push_back(entity);

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
	for (auto entity : rasterEntities)
	{
		delete entity;
	}

	for (auto entity : shapeEntities)
	{
		delete entity; 
	}

	rasterEntities.clear();
	shapeEntities.clear(); 
}