#pragma once

#include "config.h"

class Scene
{
public:
	Scene();

	std::vector<glm::vec3> trianglePositions;
	std::vector<glm::vec3> pentagonPositions;
	std::vector<glm::vec3> hexagonPositions;
};