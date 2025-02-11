#pragma once

#include "config.h"
#include "Entity.h"

class Scene
{
public:
	Scene();

	std::vector<REntity> entities;
};