#include "scene.h"

Scene::Scene()
{
	for (int x = -10; x < 10; x += 2)
	{
		for (int y = -10; y < 10; y += 2)
		{
			trianglePositions.push_back(glm::vec3(x * 0.1f, y * 0.1f, 0.0f));
		}
	}
}