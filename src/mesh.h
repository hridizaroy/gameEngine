#pragma once

#include "config.h"
#include "Material.h"

struct Mesh
{
public:
	std::shared_ptr<Material> material;

private:
	std::vector<float> vertexData;
	std::vector<int> indexData;
};