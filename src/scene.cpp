#include "scene.h"

Scene::Scene()
{
	for (int y = -10; y < 10; y += 2)
	{
		trianglePositions.push_back(glm::vec3(-0.4f, y * 0.1f, 0.0f));
		pentagonPositions.push_back(glm::vec3(0.0f, y * 0.1f, 0.0f));
		hexagonPositions.push_back(glm::vec3(0.1f, y * 0.1f, 0.0f));
	}
}