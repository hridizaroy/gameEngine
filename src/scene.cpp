#include "scene.h"

Scene::Scene()
{
	// Generate raster entities 
	for (int y = -10; y < 10; y += 2)
	{
		REntity entity;
	
		std::shared_ptr<UInfo> info = std::make_shared<UInfo>();
		info->name = "ID: " + std::to_string(y);
		info->transform = std::make_shared<Transform>();
		info->transform->MoveAbs(glm::vec3(-0.4f, y * 0.1f, 0.0f));
	
		entity.info = info;
		entity.meshType = TRIANGLE;
	
		entities.push_back(entity);
	}

	//{
	//	REntity entity;
	//
	//	std::shared_ptr<UInfo> info = std::make_shared<UInfo>();
	//	info->name = "ID: Fullscreen";
	//	info->transform = std::make_shared<Transform>();
	//
	//	entity.info = info;
	//	entity.meshType = TRIANGLE_FULLSCREEN;
	//
	//	entities.push_back(entity);
	//
	//}



}